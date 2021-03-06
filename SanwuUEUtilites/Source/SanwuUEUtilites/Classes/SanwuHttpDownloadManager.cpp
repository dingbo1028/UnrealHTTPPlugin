#include "SanwuUEUtilitesPrivatePCH.h"
#include "SanwuHttpDownloadManager.h"
#include "Http.h"

DEFINE_LOG_CATEGORY_STATIC(LogHttpDownload, Warning, All);

UHttpDownloadManager* UHttpDownloadManager::Manager;

UHttpDownloadManager::UHttpDownloadManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UHttpDownloadManager* UHttpDownloadManager::DownloadFile(FString url, FString FileName, FString Dirctory)
{
	UHttpDownloadManager* Downloader = NewObject<UHttpDownloadManager>();
	Downloader->url = url;
	Downloader->fileName = FileName;
	Downloader->DirctoryPath = Dirctory;
	if (IFileManager::Get().DirectoryExists(*(FPaths::GameSavedDir() / "Download/" + Dirctory)) ==false)
	{
		FString NewDirectorPath = FPaths::GameSavedDir() / "Download/" + Dirctory + "/";
		IFileManager::Get().MakeDirectory(*NewDirectorPath,true);
	}
	if (url.Len()>0&&FileName.Len()>0)
	{
		Downloader->GetFileSize();
	}
	else {
		Downloader->OnFail.Broadcast("URL and FileName must set",0);
	}
	Manager = Downloader;
	return Downloader;
}
void UHttpDownloadManager::GetFileSize()
{
	TSharedRef<class IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UHttpDownloadManager::HandleGetFileSize);
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetHeader("Range", "bytes=0-1");
	HttpRequest->ProcessRequest();
}
void UHttpDownloadManager::HandleGetFileSize(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	int32 ResponseCode = HttpResponse->GetResponseCode();
	if (ResponseCode/200==1)
	{
		FString ContentRangeStr=HttpResponse->GetHeader("Content-Range");
		FString SplitLeftStr;
		FString SplitRightStr;
		ContentRangeStr.Split("/", &SplitLeftStr, &SplitRightStr);
		if (SplitRightStr.Len()>0)
		{
			FileTotalSize =FCString::Atoi(*SplitRightStr);
			//FileTotalSize = 201;
			//创建任务文件
			FILE *Stream;
			if (
				fopen_s(&Stream,TCHAR_TO_UTF8( *GetDestinationPath()),"wb+")==0
				) {
				fseek(Stream, FileTotalSize - 1, SEEK_SET);
				fputc(EOF, Stream);
				fclose(Stream);
			}
			//填充任务池
 			MissionPool.Empty(); //1024*500=512000  500KB一个任务
			if (FileTotalSize>512000)
			{
				int32 MissonNum = FileTotalSize / 512000;
				MissonNum++;
				for (int32 k = 0; k < MissonNum; k++)
				{
					int32 StartPoint = k * 512000;
					int32 EndPoint = StartPoint + 512000 - 1;
					FString StartStr = FString::FromInt(StartPoint);
					FString EndStr;
					if (EndPoint > FileTotalSize)
					{
						EndStr = "";
					}
					else {
						EndStr = FString::FromInt(EndPoint);
					}
					MissionPool.Add("bytes="+StartStr + "-" + EndStr);
				}
			}
			else {
				MissionPool.Add("bytes=0-");
			}
		}
// 		MissionPool.Empty();
// 		MissionPool.Add("bytes=0-99");
// 		MissionPool.Add("bytes=100-199");

		//创建下载线程
		for (int32 d = 0; d < 10;d++)
		{
			UHttpDownloader::Init(Manager,d);
		}
		
	}
}
FString UHttpDownloadManager::GetMissionURL()
{
	return url;
}
FString UHttpDownloadManager::GetMissonContent()
{
	if (MissionPool.Num()<1)
	{
		return "";
	}

	FString Misson = MissionPool[0];
	MissionPool.RemoveAt(0);
	return Misson;
}
FString UHttpDownloadManager::GetDestinationPath()
{
	return FPaths::GameSavedDir() / "Download/" + DirctoryPath / fileName;
}
void UHttpDownloadManager::HandlePieceDownload(int32 DownloadedByte)
{
	FileDownloadedSize += DownloadedByte;
	OnProgress.Broadcast(DestinationPath, FileDownloadedSize*1.0 / FileTotalSize);
	if (FileDownloadedSize==FileTotalSize)
	{
		OnSuccess.Broadcast(GetDestinationPath(), 1);
	}
}
void UHttpDownloadManager::SaveByteToFile(FHttpRequestPtr HttpRequest, int32 UploadTotal, int32 DownloadTotal)
{
	//Todo 
	if (FFileHelper::SaveArrayToFile(HttpRequest->GetResponse()->GetContent(), 
		*(FPaths::GameSavedDir() / "Download/" + DirctoryPath / fileName))
		)
	{

	}
	
}
