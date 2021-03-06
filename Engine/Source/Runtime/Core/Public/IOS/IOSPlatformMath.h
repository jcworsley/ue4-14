// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

/*=============================================================================================
	IOSPlatformMath.h: IOS platform Math functions
==============================================================================================*/

#pragma once
#include "GenericPlatform/GenericPlatformMath.h"

/**
* IOS implementation of the Math OS functions
**/
struct FIOSPlatformMath : public FGenericPlatformMath
{
	/**
	 * Counts the number of leading zeros in the bit representation of the value
	 *
	 * @param Value the value to determine the number of leading zeros for
	 *
	 * @return the number of zeros before the first "on" bit
	 */
	static FORCEINLINE uint32 CountLeadingZeros(uint32 Value)
	{
		if (Value == 0)
		{
			return 32;
		}
	
		return __builtin_clz(Value);
	}
	
	/**
	 * Counts the number of trailing zeros in the bit representation of the value
	 *
	 * @param Value the value to determine the number of trailing zeros for
	 *
	 * @return the number of zeros after the last "on" bit
	 */
	static FORCEINLINE uint32 CountTrailingZeros(uint32 Value)
	{
		if (Value == 0)
		{
			return 32;
		}
	
		return __builtin_ctz(Value);
	}
};

typedef FIOSPlatformMath FPlatformMath;
