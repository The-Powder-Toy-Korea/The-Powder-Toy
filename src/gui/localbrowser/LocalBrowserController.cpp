#include "LocalBrowserController.h"

#include "LocalBrowserModel.h"
#include "LocalBrowserView.h"

#include "client/Client.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "tasks/TaskWindow.h"
#include "tasks/Task.h"

#include "Controller.h"

#include "common/tpt-minmax.h"

LocalBrowserController::LocalBrowserController(std::function<void ()> onDone_):
	HasDone(false)
{
	browserModel = new LocalBrowserModel();
	browserView = new LocalBrowserView();
	browserView->AttachController(this);
	browserModel->AddObserver(browserView);

	onDone = onDone_;

	browserModel->UpdateSavesList(1);
}

void LocalBrowserController::OpenSave(SaveFile * save)
{
	browserModel->SetSave(save);
}

SaveFile * LocalBrowserController::GetSave()
{
	return browserModel->GetSave();
}

void LocalBrowserController::RemoveSelected()
{
	StringBuilder desc;
	desc << browserModel->GetSelected().size() << "개의 스탬프를 제거하시겠습니까?";
	new ConfirmPrompt("Delete stamps", desc.Build(), { [this] { removeSelectedC(); } });
}

void LocalBrowserController::removeSelectedC()
{
	class RemoveSavesTask : public Task
	{
		std::vector<ByteString> saves;
		LocalBrowserController * c;
	public:
		RemoveSavesTask(LocalBrowserController * c, std::vector<ByteString> saves_) : c(c) { saves = saves_; }
		bool doWork() override
		{
			for (size_t i = 0; i < saves.size(); i++)
			{
				notifyStatus(String::Build("Deleting stamp [", saves[i].FromUtf8(), "] ..."));
				Client::Ref().DeleteStamp(saves[i]);
				notifyProgress((i + 1) * 100 / saves.size());
			}
			return true;
		}
		void after() override
		{
			Client::Ref().updateStamps();
			c->RefreshSavesList();
		}
	};

	std::vector<ByteString> selected = browserModel->GetSelected();
	new TaskWindow("스탬프를 제거하는 중", new RemoveSavesTask(this, selected));
}

void LocalBrowserController::RescanStamps()
{
	new ConfirmPrompt("다시 검사", "스탬프 디렉토리를 다시 검사하면 새로 추가된 스탬프를 찾거나 stamps.def 파일이 훼손되었을 때 이를 복구할 수 있습니다. 하지만 현재 정렬된 스탬프가 어지럽혀질 수도 있습니다.", { [this] { rescanStampsC(); } });
}

void LocalBrowserController::rescanStampsC()
{
	browserModel->RescanStamps();
	browserModel->UpdateSavesList(browserModel->GetPageNum());
}

void LocalBrowserController::RefreshSavesList()
{
	ClearSelection();
	browserModel->UpdateSavesList(browserModel->GetPageNum());
}

void LocalBrowserController::ClearSelection()
{
	browserModel->ClearSelected();
}

void LocalBrowserController::SetPage(int page)
{
	if (page != browserModel->GetPageNum() && page > 0 && page <= browserModel->GetPageCount())
		browserModel->UpdateSavesList(page);
}

void LocalBrowserController::SetPageRelative(int offset)
{
	int page = std::min(std::max(browserModel->GetPageNum() + offset, 1), browserModel->GetPageCount());
	if (page != browserModel->GetPageNum())
		browserModel->UpdateSavesList(page);
}

void LocalBrowserController::Update()
{
	if(browserModel->GetSave())
	{
		Exit();
	}
}

void LocalBrowserController::Selected(ByteString saveName, bool selected)
{
	if(selected)
		browserModel->SelectSave(saveName);
	else
		browserModel->DeselectSave(saveName);
}

bool LocalBrowserController::GetMoveToFront()
{
	return browserModel->GetMoveToFront();
}

void LocalBrowserController::SetMoveToFront(bool move)
{
	browserModel->SetMoveToFront(move);
}

void LocalBrowserController::Exit()
{
	browserView->CloseActiveWindow();
	if (onDone)
		onDone();
	HasDone = true;
}

LocalBrowserController::~LocalBrowserController()
{
	browserView->CloseActiveWindow();
	delete browserModel;
	delete browserView;
}

