#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SanwuFileManager.generated.h"
using namespace UF;
UCLASS()
class USanwuFileManager :public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**移动文件位置*/
	UFUNCTION(BlueprintCallable, Category = "Sanwu|FileManager")
	static bool MoveFileTo(FString To, FString From);

	/**删除一个文件*/
	UFUNCTION(BlueprintCallable, Category = "Sanwu|FileManager")
	static bool DeleteFile(FString FilePath);
};