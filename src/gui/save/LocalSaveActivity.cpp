#include "LocalSaveActivity.h"

#include "client/Client.h"
#include "client/GameSave.h"
#include "client/ThumbnailRendererTask.h"
#include "common/platform/Platform.h"
#include "graphics/Graphics.h"
#include "graphics/VideoBuffer.h"
#include "gui/Style.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"

#include "Config.h"

LocalSaveActivity::LocalSaveActivity(std::unique_ptr<SaveFile> newSave, OnSaved onSaved_) :
	WindowActivity(ui::Point(-1, -1), ui::Point(220, 200)),
	save(std::move(newSave)),
	thumbnailRenderer(nullptr),
	onSaved(onSaved_)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 16), "로컬 드라이브에 저장:");
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	filenameField = new ui::Textbox(ui::Point(8, 25), ui::Point(Size.X-16, 16), save->GetDisplayName(), "파일 이름");
	filenameField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	filenameField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(filenameField);
	FocusComponent(filenameField);

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X-75, 16), "취소");
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	cancelButton->SetActionCallback({ [this] {
		Exit();
	} });
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X-76, Size.Y-16), ui::Point(76, 16), "저장");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::InformationTitle;
	okayButton->SetActionCallback({ [this] {
		Save();
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	if(save->GetGameSave())
	{
		thumbnailRenderer = new ThumbnailRendererTask(*save->GetGameSave(), Size - Vec2(16, 16), RendererSettings::decorationEnabled, false);
		thumbnailRenderer->Start();
	}
}

void LocalSaveActivity::OnTick(float dt)
{
	if (thumbnailRenderer)
	{
		thumbnailRenderer->Poll();
		if (thumbnailRenderer->GetDone())
		{
			thumbnail = thumbnailRenderer->Finish();
			thumbnailRenderer = nullptr;
		}
	}
}

void LocalSaveActivity::Save()
{
	if (filenameField->GetText().Contains('/') || filenameField->GetText().BeginsWith("."))
	{
		new ErrorMessage("오류", "파일 이름이 잘못되었습니다.");
	}
	else if (filenameField->GetText().length())
	{
		ByteString finalFilename = ByteString::Build(LOCAL_SAVE_DIR, PATH_SEP_CHAR, filenameField->GetText().ToUtf8(), ".cps");
		save->SetDisplayName(filenameField->GetText());
		save->SetFileName(finalFilename);
		if (Platform::FileExists(finalFilename))
		{
			new ConfirmPrompt("파일 덮어쓰기", "파일 "+finalFilename.FromUtf8()+"\n을(를) 정말로 덮어쓰시겠습니까?", { [this, finalFilename] {
				saveWrite(finalFilename);
			} });
		}
		else
		{
			saveWrite(finalFilename);
		}
	}
	else
	{
		new ErrorMessage("오류", "파일 이름을 입력하십시오.");
	}
}

void LocalSaveActivity::saveWrite(ByteString finalFilename)
{
	Platform::MakeDirectory(LOCAL_SAVE_DIR);
	Json::Value localSaveInfo;
	localSaveInfo["type"] = "localsave";
	localSaveInfo["username"] = Client::Ref().GetAuthUser().Username;
	localSaveInfo["title"] = finalFilename;
	localSaveInfo["date"] = (Json::Value::UInt64)time(nullptr);
	Client::Ref().SaveAuthorInfo(&localSaveInfo);
	{
		auto gameSave = save->TakeGameSave();
		gameSave->authors = localSaveInfo;
		save->SetGameSave(std::move(gameSave));
	}
	std::vector<char> saveData;
	std::tie(std::ignore, saveData) = save->GetGameSave()->Serialise();
	if (saveData.size() == 0)
		new ErrorMessage("오류", "게임 데이터를 시리얼화할 수 없습니다.");
	else if (!Platform::WriteFile(saveData, finalFilename))
		new ErrorMessage("오류", "세이브 파일을 쓸 수 없습니다.");
	else
	{
		if (onSaved)
		{
			onSaved(std::move(save));
		}
		Exit();
	}
}

void LocalSaveActivity::OnDraw()
{
	Graphics * g = GetGraphics();
	g->BlendRGBAImage(saveToDiskImage->data(), RectSized(Vec2(0, 0), saveToDiskImage->Size()));
	g->DrawFilledRect(RectSized(Position, Size).Inset(-1), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);

	if (thumbnail)
	{
		auto rect = RectSized(Position + Vec2((Size.X - thumbnail->Size().X) / 2, 45), thumbnail->Size());
		g->BlendImage(thumbnail->Data(), 0xFF, rect);
		g->DrawRect(rect, 0xB4B4B4_rgb);
	}
}

LocalSaveActivity::~LocalSaveActivity()
{
	if (thumbnailRenderer)
	{
		thumbnailRenderer->Abandon();
	}
}
