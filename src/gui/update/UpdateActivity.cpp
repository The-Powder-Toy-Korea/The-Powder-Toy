#include "UpdateActivity.h"
#include "client/http/Request.h"
#include "prefs/GlobalPrefs.h"
#include "client/Client.h"
#include "common/platform/Platform.h"
#include "tasks/Task.h"
#include "tasks/TaskWindow.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Engine.h"
#include "Config.h"
#include <bzlib.h>
#include <memory>

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
		auto &prefs = GlobalPrefs::Ref();

		auto niceNotifyError = [this](String error) {
			notifyError("Downloaded update is corrupted\n" + error);
			return false;
		};

		auto request = std::make_unique<http::Request>(updateName);
		request->Start();
		notifyStatus("업데이트를 내려받는 중");
		notifyProgress(-1);
		while(!request->CheckDone())
		{
			int total, done;
			std::tie(total, done) = request->CheckProgress();
			notifyProgress(total ? done * 100 / total : 0);
			Platform::Millisleep(1);
		}

		auto [ status, data ] = request->Finish();
		if (status!=200)
		{
			return niceNotifyError("업데이트를 내려받을 수 없음: " + String::Build("서버가 다음 코드로 응답함: Status ", status));
		}
		if (!data.size())
		{
			return niceNotifyError("서버가 데이터를 반환하지 않음");
		}

		notifyStatus("업데이트를 언팩하는 중");
		notifyProgress(-1);

		unsigned int uncompressedLength;

		if(data.size()<16)
		{
			return niceNotifyError(String::Build("충분하지 않은 데이터: ", data.size(), " 바이트"));
		}
		if (data[0]!=0x42 || data[1]!=0x75 || data[2]!=0x54 || data[3]!=0x54)
		{
			return niceNotifyError("알 수 없는 업데이트 포맷");
		}

		uncompressedLength  = (unsigned char)data[4];
		uncompressedLength |= ((unsigned char)data[5])<<8;
		uncompressedLength |= ((unsigned char)data[6])<<16;
		uncompressedLength |= ((unsigned char)data[7])<<24;

		std::vector<char> res(uncompressedLength);

		int dstate;
		dstate = BZ2_bzBuffToBuffDecompress(&res[0], (unsigned *)&uncompressedLength, &data[8], data.size()-8, 0, 0);
		if (dstate)
		{
			return niceNotifyError(String::Build("업데이트 파일을 압축 해제할 수 없음: ", dstate));
		}

		notifyStatus("업데이트를 적용하는 중");
		notifyProgress(-1);

		prefs.Set("version.update", true);
		if (!Platform::UpdateStart(res))
		{
			prefs.Set("version.update", false);
			Platform::UpdateCleanup();
			notifyError("업데이트에 실패함 - 더 높은 버전의 The Powder Toy가 필요합니다.");
			return false;
		}

		return true;
	}
};

UpdateActivity::UpdateActivity() {
	ByteString file = ByteString::Build(SCHEME, USE_UPDATESERVER ? UPDATESERVER : SERVER, Client::Ref().GetUpdateInfo().File);
	updateDownloadTask = new UpdateDownloadTask(file, this);
	updateWindow = new TaskWindow("업데이트를 내려받는 중...", updateDownloadTask, true);
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
	StringBuilder sb;
	if constexpr (USE_UPDATESERVER)
	{
		sb << "Please go online to manually download a newer version.\n";
	}
	else
	{
		sb << "The Powder Toy 공식 홈페이지에서 게임을 내려받으십시오.\n";
	}
	sb << "Error: " << sender->GetError();
	new ConfirmPrompt("자동 업데이트에 실패함", sb.Build(), { [this] {
		if constexpr (!USE_UPDATESERVER)
		{
			Platform::OpenURI(ByteString(SCHEME) + "powdertoy.co.uk/Download.html");
		}
		Exit();
	}, [this] { Exit(); } });
}


UpdateActivity::~UpdateActivity() {
}
