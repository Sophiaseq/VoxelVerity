// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeInfo.h"

#include "MGLogChannels.h"

FMGAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound)
{
	for (FMGAttributeInfo& Info : AttributeInformation)
	{
		if (Info.AttributeTag == AttributeTag)
		{
			return Info;
		}
	}
	
	if (bLogNotFound)
	{
		UE_LOG(LogMG, Error, TEXT("没有在AttributeInfo:[%s]中找到关于[%s]标签的信息"), *GetNameSafe(this), *AttributeTag.ToString());
	}
	
	return FMGAttributeInfo();
}
