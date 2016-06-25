﻿// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "BlueprintGraphPrivatePCH.h"
#include "KismetCompiler.h"
#include "MakeStructHandler.h"
#include "BlueprintFieldNodeSpawner.h"
#include "EditorCategoryUtils.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintActionFilter.h"	// for FBlueprintActionContext
#include "Editor/PropertyEditor/Public/PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "K2Node_MakeStruct"

//////////////////////////////////////////////////////////////////////////
// UK2Node_MakeStruct


UK2Node_MakeStruct::FMakeStructPinManager::FMakeStructPinManager(const uint8* InSampleStructMemory)
	: FStructOperationOptionalPinManager(), SampleStructMemory(InSampleStructMemory)
{
}

void UK2Node_MakeStruct::FMakeStructPinManager::CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex, UProperty* Property) const
{
	struct FPinDefaultValueHelper
	{
		static void Set(UEdGraphPin& InPin, const FString& Value, bool bIsText, bool bIsObject)
		{
			InPin.AutogeneratedDefaultValue = Value;
			if (bIsText)
			{
				if (!FTextStringHelper::ReadFromString(*Value, InPin.DefaultTextValue))
				{
					InPin.DefaultTextValue = FText::FromString(Value);
				}
			}
			else if (bIsObject)
			{
				InPin.DefaultObject = StaticFindObject(UObject::StaticClass(), nullptr, *Value);
			}
			else
			{
				InPin.DefaultValue = Value;
			}
		}
	};

	UK2Node_StructOperation::FStructOperationOptionalPinManager::CustomizePinData(Pin, SourcePropertyName, ArrayIndex, Property);
	if (Pin && Property)
	{
		const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
		check(Schema);

		// Should pin default value be filled as FText?
		const bool bIsText = Property->IsA<UTextProperty>();
		checkSlow(bIsText == ((Schema->PC_Text == Pin->PinType.PinCategory) && !Pin->PinType.bIsArray));

		const bool bIsObject = Property->IsA<UObjectPropertyBase>();
		checkSlow(bIsObject == ((Schema->PC_Object == Pin->PinType.PinCategory || Schema->PC_Class == Pin->PinType.PinCategory || 
			Schema->PC_Asset == Pin->PinType.PinCategory || Schema->PC_AssetClass == Pin->PinType.PinCategory) && !Pin->PinType.bIsArray));

		const FString& MetadataDefaultValue = Property->GetMetaData(TEXT("MakeStructureDefaultValue"));
		if (!MetadataDefaultValue.IsEmpty())
		{
			FPinDefaultValueHelper::Set(*Pin, MetadataDefaultValue, bIsText, bIsObject);
			return;
		}

		if (NULL != SampleStructMemory)
		{
			FString NewDefaultValue;
			if (Property->ExportText_InContainer(0, NewDefaultValue, SampleStructMemory, SampleStructMemory, NULL, PPF_None))
			{
				if (Schema->IsPinDefaultValid(Pin, NewDefaultValue, NULL, FText::GetEmpty()).IsEmpty())
				{
					FPinDefaultValueHelper::Set(*Pin, NewDefaultValue, bIsText, bIsObject);
					return;
				}
			}
		}

		Schema->SetPinDefaultValueBasedOnType(Pin);
	}
}

static bool CanBeExposed(const UProperty* Property, bool bIncludeEditOnly = true)
{
	if (Property)
	{
		const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
		check(Schema);

		FEdGraphPinType DumbGraphPinType;
		const bool bConvertable = Schema->ConvertPropertyToPinType(Property, /*out*/ DumbGraphPinType);

		//TODO: remove CPF_Edit in a future release
		const bool bVisible = (bIncludeEditOnly ? Property->HasAnyPropertyFlags(CPF_BlueprintVisible | CPF_Edit) : Property->HasAnyPropertyFlags(CPF_BlueprintVisible)) && !(Property->ArrayDim > 1);
		const bool bBlueprintReadOnly = Property->HasAllPropertyFlags(CPF_BlueprintReadOnly);
		if (bVisible && bConvertable && !bBlueprintReadOnly)
		{
			return true;
		}
	}
	return false;
}

bool UK2Node_MakeStruct::FMakeStructPinManager::CanTreatPropertyAsOptional(UProperty* TestProperty) const
{
	return CanBeExposed(TestProperty);
}

UK2Node_MakeStruct::UK2Node_MakeStruct(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bMadeAfterOverridePinRemoval(false)
{
}

void UK2Node_MakeStruct::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
	if(Schema && StructType)
	{
		CreatePin(EGPD_Output, Schema->PC_Struct, TEXT(""), StructType, false, false, StructType->GetName());
		
		{
			FStructOnScope StructOnScope(Cast<UScriptStruct>(StructType));
			FMakeStructPinManager OptionalPinManager(StructOnScope.GetStructMemory());
			OptionalPinManager.RebuildPropertyList(ShowPinForProperties, StructType);
			OptionalPinManager.CreateVisiblePins(ShowPinForProperties, StructType, EGPD_Input, this);
		}

		// When struct has a lot of fields, mark their pins as advanced
		if(Pins.Num() > 5)
		{
			if(ENodeAdvancedPins::NoPins == AdvancedPinDisplay)
			{
				AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
			}

			for(int32 PinIndex = 3; PinIndex < Pins.Num(); ++PinIndex)
			{
				if(UEdGraphPin * EdGraphPin = Pins[PinIndex])
				{
					EdGraphPin->bAdvancedView = true;
				}
			}
		}
	}
}

void UK2Node_MakeStruct::ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const
{
	if(!StructType)
	{
		MessageLog.Error(*LOCTEXT("NoStruct_Error", "No Struct in @@").ToString(), this);
	}
	else
	{
		bool bHasAnyBlueprintVisibleProperty = false;
		for (TFieldIterator<UProperty> It(StructType); It; ++It)
		{
			const UProperty* Property = *It;
			if (CanBeExposed(Property))
			{
				const bool bIsBlueprintVisible = Property->HasAnyPropertyFlags(CPF_BlueprintVisible) || (Property->GetOwnerStruct() && Property->GetOwnerStruct()->IsA<UUserDefinedStruct>());
				bHasAnyBlueprintVisibleProperty |= bIsBlueprintVisible;

				const UEdGraphPin* Pin = FindPin(Property->GetName());

				if (Pin && !bIsBlueprintVisible)
				{
					MessageLog.Warning(*LOCTEXT("PropertyIsNotBPVisible_Warning", "@@ - the native property is not tagged as BlueprintReadWrite, the pin will be removed in a future release.").ToString(), Pin);
				}

				if (Property->ArrayDim > 1)
				{
					MessageLog.Warning(*LOCTEXT("StaticArray_Warning", "@@ - the native property is a static array, which is not supported by blueprints").ToString(), Pin);
				}
			}
		}

		if (!bHasAnyBlueprintVisibleProperty)
		{
			MessageLog.Warning(*LOCTEXT("StructHasNoBPVisibleProperties_Warning", "@@ has no property tagged as BlueprintReadWrite. The node will be removed in a future release.").ToString(), this);
		}

		if (!bMadeAfterOverridePinRemoval)
		{
			MessageLog.Note(*NSLOCTEXT("K2Node", "OverridePinRemoval_SetFieldsInStruct", "Override pins have been removed from @@ in @@, it functions the same as it did but some functionality may be deprecated! This note will go away after you resave the asset!").ToString(), this, GetBlueprint());
		}
	}
}


FText UK2Node_MakeStruct::GetNodeTitle(ENodeTitleType::Type TitleType) const 
{
	if (StructType == nullptr)
	{
		return LOCTEXT("MakeNullStructTitle", "Make <unknown struct>");
	}
	else if (CachedNodeTitle.IsOutOfDate(this))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("StructName"), FText::FromName(StructType->GetFName()));
		// FText::Format() is slow, so we cache this to save on performance
		CachedNodeTitle.SetCachedText(FText::Format(LOCTEXT("MakeNodeTitle", "Make {StructName}"), Args), this);
	}
	return CachedNodeTitle;
}

FText UK2Node_MakeStruct::GetTooltipText() const
{
	if (StructType == nullptr)
	{
		return LOCTEXT("MakeNullStruct_Tooltip", "Adds a node that create an '<unknown struct>' from its members");
	}
	else if (CachedTooltip.IsOutOfDate(this))
	{
		// FText::Format() is slow, so we cache this to save on performance
		CachedTooltip.SetCachedText(FText::Format(
			LOCTEXT("MakeStruct_Tooltip", "Adds a node that create a '{0}' from its members"),
			FText::FromName(StructType->GetFName())
		), this);
	}
	return CachedTooltip;
}

FSlateIcon UK2Node_MakeStruct::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.MakeStruct_16x");
	return Icon;
}

FLinearColor UK2Node_MakeStruct::GetNodeTitleColor() const
{
	if(const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>())
	{
		FEdGraphPinType PinType;
		PinType.PinCategory = K2Schema->PC_Struct;
		PinType.PinSubCategoryObject = StructType;
		return K2Schema->GetPinTypeColor(PinType);
	}
	return UK2Node::GetNodeTitleColor();
}

bool UK2Node_MakeStruct::CanBeMade(const UScriptStruct* Struct, bool bIncludeEditOnly, bool bMustHaveValidProperties )
{
	if (Struct && !Struct->HasMetaData(TEXT("HasNativeMake")))
	{
		if (!bMustHaveValidProperties && UEdGraphSchema_K2::IsAllowableBlueprintVariableType(Struct))
		{
			return true;
		}

		for (TFieldIterator<UProperty> It(Struct); It; ++It)
		{
			if (CanBeExposed(*It, bIncludeEditOnly))
			{
				return true;
			}
		}
	}
	return false;
}

FNodeHandlingFunctor* UK2Node_MakeStruct::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new FKCHandler_MakeStruct(CompilerContext);
}

UK2Node::ERedirectType UK2Node_MakeStruct::DoPinsMatchForReconstruction(const UEdGraphPin* NewPin, int32 NewPinIndex, const UEdGraphPin* OldPin, int32 OldPinIndex)  const
{
	ERedirectType Result = UK2Node::DoPinsMatchForReconstruction(NewPin, NewPinIndex, OldPin, OldPinIndex);
	if ((ERedirectType_None == Result) && DoRenamedPinsMatch(NewPin, OldPin, false))
	{
		Result = ERedirectType_Custom;
	}
	else if ((ERedirectType_None == Result) && NewPin && OldPin)
	{
		if ((EGPD_Output == NewPin->Direction) && (EGPD_Output == OldPin->Direction))
		{
			const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
			if (K2Schema->ArePinTypesCompatible(NewPin->PinType, OldPin->PinType))
			{
				Result = ERedirectType_Custom;
			}
		}
		else if ((EGPD_Input == NewPin->Direction) && (EGPD_Input == OldPin->Direction))
		{
			TMap<FName, FName>* StructRedirects = UStruct::TaggedPropertyRedirects.Find(StructType->GetFName());
			if (StructRedirects)
			{
				FName* PropertyRedirect = StructRedirects->Find(FName(*OldPin->PinName));
				if (PropertyRedirect)
				{
					Result = ((FCString::Stricmp(*PropertyRedirect->ToString(), *NewPin->PinName) != 0) ? ERedirectType_None : ERedirectType_Name);
				}
			}
		}
	}
	return Result;
}

void UK2Node_MakeStruct::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	struct GetMenuActions_Utils
	{
		static void SetNodeStruct(UEdGraphNode* NewNode, UField const* /*StructField*/, TWeakObjectPtr<UScriptStruct> NonConstStructPtr)
		{
			UK2Node_MakeStruct* MakeNode = CastChecked<UK2Node_MakeStruct>(NewNode);
			MakeNode->StructType = NonConstStructPtr.Get();
		}

		static void OverrideCategory(FBlueprintActionContext const& Context, IBlueprintNodeBinder::FBindingSet const& /*Bindings*/, FBlueprintActionUiSpec* UiSpecOut, TWeakObjectPtr<UScriptStruct> StructPtr)
		{
			for (UEdGraphPin* Pin : Context.Pins)
			{
				UScriptStruct* PinStruct = Cast<UScriptStruct>(Pin->PinType.PinSubCategoryObject.Get());
				if ((PinStruct != nullptr) && (StructPtr.Get() == PinStruct) && (Pin->Direction == EGPD_Input))
				{
					UiSpecOut->Category = LOCTEXT("EmptyCategory", "|");
					break;
				}
			}
		}
	};

	UClass* NodeClass = GetClass();
	ActionRegistrar.RegisterStructActions( FBlueprintActionDatabaseRegistrar::FMakeStructSpawnerDelegate::CreateLambda([NodeClass](const UScriptStruct* Struct)->UBlueprintNodeSpawner*
	{
		UBlueprintFieldNodeSpawner* NodeSpawner = nullptr;
		
		if (UK2Node_MakeStruct::CanBeMade(Struct, /*bIncludeEditOnly =*/false))
		{
			NodeSpawner = UBlueprintFieldNodeSpawner::Create(NodeClass, Struct);
			check(NodeSpawner != nullptr);
			TWeakObjectPtr<UScriptStruct> NonConstStructPtr = Struct;
			NodeSpawner->SetNodeFieldDelegate     = UBlueprintFieldNodeSpawner::FSetNodeFieldDelegate::CreateStatic(GetMenuActions_Utils::SetNodeStruct, NonConstStructPtr);
			NodeSpawner->DynamicUiSignatureGetter = UBlueprintFieldNodeSpawner::FUiSpecOverrideDelegate::CreateStatic(GetMenuActions_Utils::OverrideCategory, NonConstStructPtr);
		}
		return NodeSpawner;
	}) );
}

FText UK2Node_MakeStruct::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Struct);
}

void UK2Node_MakeStruct::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	// New nodes automatically have this set.
	bMadeAfterOverridePinRemoval = true;
}

void UK2Node_MakeStruct::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(this);

	if (Blueprint && !Ar.IsTransacting() && !HasAllFlags(RF_Transient))
	{
		if (Ar.IsLoading() && !bMadeAfterOverridePinRemoval)
		{
			// Check if this node actually requires warning the user that functionality has changed.
			bMadeAfterOverridePinRemoval = true;
			if (StructType != nullptr)
			{
				FOptionalPinManager PinManager;

				// Have to check if this node is even in danger.
				for (FOptionalPinFromProperty& PropertyEntry : ShowPinForProperties)
				{
					UProperty* Property = StructType->FindPropertyByName(PropertyEntry.PropertyName);
					bool bNegate = false;
					if (UProperty* OverrideProperty = PropertyCustomizationHelpers::GetEditConditionProperty(Property, bNegate))
					{
						bool bHadOverridePropertySeparation = false;
						for (FOptionalPinFromProperty& OverridePropertyEntry : ShowPinForProperties)
						{
							if (OverridePropertyEntry.PropertyName == OverrideProperty->GetFName())
							{
								bHadOverridePropertySeparation = true;
								break;
							}
						}

						bMadeAfterOverridePinRemoval = false;
						UEdGraphPin* Pin = FindPin(Property->GetName());

						if (bHadOverridePropertySeparation)
						{
							UEdGraphPin* OverridePin = FindPin(OverrideProperty->GetName());
							const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
							if (OverridePin)
							{
								// Override pins are always booleans
								check(OverridePin->PinType.PinCategory == Schema->PC_Boolean);
								// If the old override pin's default value was true, then the override should be marked as enabled
								PropertyEntry.bIsOverrideEnabled = OverridePin->DefaultValue.ToBool();
								// It had an override pin, so conceptually the override pin is visible
								PropertyEntry.bIsOverridePinVisible = true;
								// Because there was an override pin visible for this property, this property will be forced to have a pin
								PropertyEntry.bShowPin = true;
							}
							else
							{
								// No override pin, ensure all override bools are false
								PropertyEntry.bIsOverrideEnabled = false;
								PropertyEntry.bIsOverridePinVisible = false;
							}
						}
						else
						{
							if (Pin)
							{
								PropertyEntry.bIsOverrideEnabled = true;
								PropertyEntry.bIsOverridePinVisible = true;
							}
						}

						// If the pin for this property, which sets the property's value, does not exist then the user was not trying to set the value
						PropertyEntry.bIsSetValuePinVisible = Pin != nullptr;
					}
				}
			}
		}
		else if (Ar.IsSaving())
		{
			if (!Blueprint->bBeingCompiled)
			{
				bMadeAfterOverridePinRemoval = true;
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
