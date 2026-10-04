#include "UpdateActivity.h"
#include "client/http/Request.h"
#include "prefs/GlobalPrefs.h"
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
			notifyError("내려받은 업데이트 파일이 손상되었습니다.\n" + error);
			return false;
		};

		auto request = std::make_unique<http::Request>(updateName);
		request->Start();
		notifyStatus("업데이트 내려받는 중");
		notifyProgress(-1);
		while(!request->CheckDone())
		{
			int64_t total, done;
			std::tie(total, done) = request->CheckProgress();
			if (total == -1)
			{
				notifyProgress(-1);
			}
			else
			{
				notifyProgress(total ? done * 100 / total : 0);
			}
			Platform::Millisleep(1);
		}

		int status;
		ByteString data;
		try
		{
			std::tie(status, data) = request->Finish();
		}
		catch (const http::RequestError &ex)
		{
			return niceNotifyError("업데이트를 내려받을 수 없습니다. " + String::Build("서버에서 다음 상태 코드로 응답하였습니다: Status ", ByteString(ex.what()).FromAscii()));
		}
		if (status!=200)
		{
			return niceNotifyError("업데이트를 내려받을 수 없습니다. " + String::Build("서버에서 다음 상태 코드로 응답하였습니다: Status ", status));
		}
		if (!data.size())
		{
			return niceNotifyError("서버에서 데이터를 반환하지 않았습니다.");
		}

		notifyStatus("업데이트 압축 해제 중");
		notifyProgress(-1);

		unsigned int uncompressedLength;

		if(data.size()<16)
		{
			return niceNotifyError(String::Build("데이터가 부족합니다: ", data.size(), " 바이트 얻음"));
		}
		if (data[0]!=0x42 || data[1]!=0x75 || data[2]!=0x54 || data[3]!=0x54)
		{
			return niceNotifyError("업데이트 형식이 유효하지 않습니다.");
		}

		uncompressedLength  = (unsigned char)data[4];
		uncompressedLength |= ((unsigned char)data[5])<<8;
		uncompressedLength |= ((unsigned char)data[6])<<16;
		uncompressedLength |= ((unsigned char)data[7])<<24;

		std::vector<char> res(uncompressedLength);

		int dstate;
		dstate = BZ2_bzBuffToBuffDecompress(res.data(), (unsigned *)&uncompressedLength, &data[8], data.size()-8, 0, 0);
		if (dstate)
		{
			return niceNotifyError(String::Build("업데이트 파일을 압축 해제할 수 없습니다: ", dstate));
		}

		notifyStatus("업데이트 적용 중");
		notifyProgress(-1);

		prefs.Set("version.update", true);
		if (!Platform::UpdateStart(res))
		{
			prefs.Set("version.update", false);
			Platform::UpdateCleanup();
			notifyError("업데이트에 실패하였습니다. 새 버전을 직접 내려받으십시오.");
			return false;
		}

		return true;
	}
};

UpdateActivity::UpdateActivity(UpdateInfo info)
{
	updateDownloadTask = new UpdateDownloadTask(info.file, this);
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
		sb << "인터넷에 접속하여 새 버전을 직접 내려받으십시오.\n";
	}
	else
	{
		sb << "The Powder Toy 공식 홈페이지에서 새 버전을 내려받으십시오.\n";
	}
	sb << "Error: " << sender->GetError();
	new ConfirmPrompt("자동 업데이트에 실패함", sb.Build(), { [this] {
		if constexpr (!USE_UPDATESERVER)
		{
			Platform::OpenURI(ByteString::Build(SERVER, "/Download.html"));
		}
		Exit();
	}, [this] { Exit(); } });
}


UpdateActivity::~UpdateActivity() {
}
