#include "client/http/Request.h" // includes curl.h, needs to come first to silence a warning on windows

#include "UpdateActivity.h"

#include "bzip2/bzlib.h"

#include "Config.h"
#include "Update.h"

#include "client/Client.h"
#include "common/Platform.h"
#include "tasks/Task.h"
#include "tasks/TaskWindow.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Engine.h"

class UpdateDownloadTask : public Task
{
public:
	UpdateDownloadTask(ByteString updateName, UpdateActivity * a) : a(a), updateName(updateName) {}
private:
	UpdateActivity * a;
	ByteString updateName;
	void notifyDoneMain() override {
		a->NotifyDone(this);
	}
	void notifyErrorMain() override
	{
		a->NotifyError(this);
	}
	bool doWork() override
	{
		String error;
		http::Request *request = new http::Request(updateName);
		request->Start();
		notifyStatus("업데이트를 다운로드하는 중");
		notifyProgress(-1);
		while(!request->CheckDone())
		{
			int total, done;
			request->CheckProgress(&total, &done);
			notifyProgress(total ? done * 100 / total : 0);
			Platform::Millisleep(1);
		}

		int status;
		ByteString data = request->Finish(&status);
		if (status!=200)
		{
			error = String::Build("서버가 다음 코드로 응답함: Status ", status);
			notifyError("업데이트를 다운로드할 수 없음: " + error);
			return false;
		}
		if (!data.size())
		{
			error = "서버가 응답하지 않음";
			notifyError("서버가 아무 데이터를 반환하지 않음");
			return false;
		}

		notifyStatus("업데이트를 언팩하는 중");
		notifyProgress(-1);

		unsigned int uncompressedLength;

		if(data.size()<16)
		{
			error = String::Build("충분하지 않은 데이터: ", data.size(), " 바이트");
			goto corrupt;
		}
		if (data[0]!=0x42 || data[1]!=0x75 || data[2]!=0x54 || data[3]!=0x54)
		{
			error = "알 수 없는 업데이트 포맷";
			goto corrupt;
		}

		uncompressedLength  = (unsigned char)data[4];
		uncompressedLength |= ((unsigned char)data[5])<<8;
		uncompressedLength |= ((unsigned char)data[6])<<16;
		uncompressedLength |= ((unsigned char)data[7])<<24;

		char * res;
		res = (char *)malloc(uncompressedLength);
		if (!res)
		{
			error = String::Build("압축 해제용 메모리 ", uncompressedLength, " 바이트를 할당하는 데에 실패함");
			goto corrupt;
		}

		int dstate;
		dstate = BZ2_bzBuffToBuffDecompress((char *)res, (unsigned *)&uncompressedLength, &data[8], data.size()-8, 0, 0);
		if (dstate)
		{
			error = String::Build("업데이트 파일을 압축 해제할 수 없음: ", dstate);
			free(res);
			goto corrupt;
		}

		notifyStatus("업데이트를 적용하는 중");
		notifyProgress(-1);

		Client::Ref().SetPref("version.update", true);
		if (update_start(res, uncompressedLength))
		{
			Client::Ref().SetPref("version.update", false);
			update_cleanup();
			notifyError("업데이트에 실패함 - 더 높은 버전의 The Powder Toy가 필요합니다.");
			return false;
		}

		return true;

	corrupt:
		notifyError("다운로드된 업데이트가 손상되었습니다\n" + error);
		return false;
	}
};

UpdateActivity::UpdateActivity() {
	ByteString file;
#ifdef UPDATESERVER
	file = ByteString::Build(SCHEME, UPDATESERVER, Client::Ref().GetUpdateInfo().File);
#else
	file = ByteString::Build(SCHEME, SERVER, Client::Ref().GetUpdateInfo().File);
#endif
	updateDownloadTask = new UpdateDownloadTask(file, this);
	updateWindow = new TaskWindow("업데이트를 다운로드하는 중...", updateDownloadTask, true);
}

void UpdateActivity::NotifyDone(Task * sender)
{
	if(sender->GetSuccess())
	{
		Exit();
	}
}

void UpdateActivity::Exit()
{
	updateWindow->Exit();
	ui::Engine::Ref().Exit();
	delete this;
}

void UpdateActivity::NotifyError(Task * sender)
{
#ifdef UPDATESERVER
# define FIRST_LINE "Please go online to manually download a newer version.\n"
#else
# define FIRST_LINE "The Powder Toy 공식 홈페이지에서 게임을 다운로드해 보세요.\n"
#endif
	new ConfirmPrompt("자동 업데이트에 실패함", FIRST_LINE "오류: " + sender->GetError(), { [this] {
#ifndef UPDATESERVER
		Platform::OpenURI(SCHEME "powdertoy.co.uk/Download.html");
#endif
		Exit();
	}, [this] { Exit(); } });
#undef FIRST_LINE
}


UpdateActivity::~UpdateActivity() {
}

