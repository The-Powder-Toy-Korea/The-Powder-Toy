#include "ServerSaveActivity.h"
#include "graphics/Graphics.h"
#include "graphics/VideoBuffer.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Button.h"
#include "gui/interface/Checkbox.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/SaveIDMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/InformationMessage.h"
#include "client/Client.h"
#include "client/ThumbnailRendererTask.h"
#include "client/GameSave.h"
#include "client/http/UploadSaveRequest.h"
#include "tasks/Task.h"
#include "gui/Style.h"

class SaveUploadTask: public Task
{
	SaveInfo &save;

	void before() override
	{

	}

	void after() override
	{

	}

	bool doWork() override
	{
		notifyProgress(-1);
		auto uploadSaveRequest = std::make_unique<http::UploadSaveRequest>(save);
		uploadSaveRequest->Start();
		uploadSaveRequest->Wait();
		try
		{
			save.SetID(uploadSaveRequest->Finish());
		}
		catch (const http::RequestError &ex)
		{
			notifyError(ByteString(ex.what()).FromUtf8());
			return false;
		}
		return true;
	}

public:
	SaveUploadTask(SaveInfo &newSave):
		save(newSave)
	{

	}
};

ServerSaveActivity::ServerSaveActivity(std::unique_ptr<SaveInfo> newSave, OnUploaded onUploaded_) :
	WindowActivity(ui::Point(-1, -1), ui::Point(440, 200)),
	thumbnailRenderer(nullptr),
	save(std::move(newSave)),
	onUploaded(onUploaded_),
	saveUploadTask(NULL)
{
	titleLabel = new ui::Label(ui::Point(4, 5), ui::Point((Size.X/2)-8, 16), "");
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);
	CheckName(save->GetName()); //set titleLabel text

	ui::Label * previewLabel = new ui::Label(ui::Point((Size.X/2)+4, 5), ui::Point((Size.X/2)-8, 16), "미리 보기:");
	previewLabel->SetTextColour(style::Colour::InformationTitle);
	previewLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	previewLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(previewLabel);

	nameField = new ui::Textbox(ui::Point(8, 25), ui::Point((Size.X/2)-16, 16), save->GetName(), "세이브 이름");
	nameField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	nameField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	nameField->SetActionCallback({ [this] { CheckName(nameField->GetText()); } });
	nameField->SetLimit(50);
	AddComponent(nameField);
	FocusComponent(nameField);

	descriptionField = new ui::Textbox(ui::Point(8, 65), ui::Point((Size.X/2)-16, Size.Y-(65+16+4)), save->GetDescription(), "세이브 설명");
	descriptionField->SetMultiline(true);
	descriptionField->SetLimit(254);
	descriptionField->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	descriptionField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(descriptionField);

	publishedCheckbox = new ui::Checkbox(ui::Point(8, 45), ui::Point((Size.X/2)-80, 16), "게시", "");
	if(Client::Ref().GetAuthUser().Username != save->GetUserName())
	{
		//Save is not owned by the user, disable by default
		publishedCheckbox->SetChecked(false);
	}
	else
	{
		//Save belongs to the current user, use published state already set
		publishedCheckbox->SetChecked(save->GetPublished());
	}
	AddComponent(publishedCheckbox);

	pausedCheckbox = new ui::Checkbox(ui::Point(149, 45), ui::Point(55, 16), "일시 정지", "");
	pausedCheckbox->SetChecked(save->GetGameSave()->paused);
	AddComponent(pausedCheckbox);

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point((Size.X/2)-75, 16), "취소");
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	cancelButton->SetActionCallback({ [this] {
		Exit();
	} });
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point((Size.X/2)-76, Size.Y-16), ui::Point(76, 16), "저장");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::InformationTitle;
	okayButton->SetActionCallback({ [this] {
		Save();
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	ui::Button * PublishingInfoButton = new ui::Button(ui::Point((Size.X*3/4)-75, Size.Y-42), ui::Point(150, 16), "게시 안내");
	PublishingInfoButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	PublishingInfoButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	PublishingInfoButton->Appearance.TextInactive = style::Colour::InformationTitle;
	PublishingInfoButton->SetActionCallback({ [this] {
		ShowPublishingInfo();
	} });
	AddComponent(PublishingInfoButton);

	ui::Button * RulesButton = new ui::Button(ui::Point((Size.X*3/4)-75, Size.Y-22), ui::Point(150, 16), "세이브 업로드 규칙");
	RulesButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	RulesButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	RulesButton->Appearance.TextInactive = style::Colour::InformationTitle;
	RulesButton->SetActionCallback({ [this] {
		ShowRules();
	} });
	AddComponent(RulesButton);

	if (save->GetGameSave())
	{
		thumbnailRenderer = new ThumbnailRendererTask(*save->GetGameSave(), Size / 2 - Vec2(16, 16), RendererSettings::decorationAntiClickbait, true);
		thumbnailRenderer->Start();
	}
}

ServerSaveActivity::ServerSaveActivity(std::unique_ptr<SaveInfo> newSave, bool saveNow, OnUploaded onUploaded_) :
	WindowActivity(ui::Point(-1, -1), ui::Point(200, 50)),
	thumbnailRenderer(nullptr),
	save(std::move(newSave)),
	onUploaded(onUploaded_),
	saveUploadTask(NULL)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(0, 0), Size, "서버에 저장하는 중...");
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	AddAuthorInfo();

	saveUploadTask = new SaveUploadTask(*this->save);
	saveUploadTask->AddTaskListener(this);
	saveUploadTask->Start();
}

void ServerSaveActivity::NotifyDone(Task * task)
{
	if(!task->GetSuccess())
	{
		Exit();
		new ErrorMessage("오류", task->GetError());
	}
	else
	{
		if (onUploaded)
		{
			onUploaded(std::move(save));
		}
		Exit();
	}
}

void ServerSaveActivity::Save()
{
	if (!nameField->GetText().length())
	{
		new ErrorMessage("오류", "파일 이름을 입력해야 합니다.");
		return;
	}
	if(Client::Ref().GetAuthUser().Username != save->GetUserName() && publishedCheckbox->GetChecked())
	{
		new ConfirmPrompt("게시", "이 세이브는 " + save->GetUserName().FromUtf8() + "에 의해 제작되었으며, 귀하가 이것을 귀하의 이름으로 게시하려 하고 있습니다. 게시자의 허가가 있지 않다면 게시 확인 상자를 해제하십시오. 게시자의 허가가 있다면 게시해도 됩니다.", { [this] {
			saveUpload();
		} });
	}
	else
	{
		saveUpload();
	}
}

void ServerSaveActivity::AddAuthorInfo()
{
	Json::Value serverSaveInfo;
	serverSaveInfo["type"] = "save";
	serverSaveInfo["id"] = save->GetID();
	serverSaveInfo["username"] = Client::Ref().GetAuthUser().Username;
	serverSaveInfo["title"] = save->GetName().ToUtf8();
	serverSaveInfo["description"] = save->GetDescription().ToUtf8();
	serverSaveInfo["published"] = (int)save->GetPublished();
	serverSaveInfo["date"] = (Json::Value::UInt64)time(NULL);
	Client::Ref().SaveAuthorInfo(&serverSaveInfo);
	{
		auto gameSave = save->TakeGameSave();
		gameSave->authors = serverSaveInfo;
		save->SetGameSave(std::move(gameSave));
	}
}

void ServerSaveActivity::saveUpload()
{
	okayButton->Enabled = false;
	save->SetName(nameField->GetText());
	save->SetDescription(descriptionField->GetText());
	save->SetPublished(publishedCheckbox->GetChecked());
	save->SetUserName(Client::Ref().GetAuthUser().Username);
	save->SetID(0);
	{
		auto gameSave = save->TakeGameSave();
		gameSave->paused = pausedCheckbox->GetChecked();
		save->SetGameSave(std::move(gameSave));
	}
	AddAuthorInfo();
	uploadSaveRequest = std::make_unique<http::UploadSaveRequest>(*save);
	uploadSaveRequest->Start();
}

void ServerSaveActivity::Exit()
{
	WindowActivity::Exit();
}

void ServerSaveActivity::ShowPublishingInfo()
{
	String info =
		"The Powder Toy에서는 플레이어가 자신의 계정에서 시뮬레이션을 공개하거나 공개하지 않을 수 있습니다. 이것은 '공개' 확인 상자의 체크 여부로 결정됩니다. 세이브는 기본적으로 공개되지 않으므로, 공개하지 않으면 아무도 당신의 세이브를 볼 수 없을 것입니다.\n"
		"\n"
		"\bt공개된 세이브\bw는 '날짜순 정렬'에 표시되어 많은 사람이 볼 수 있습니다. 이러한 세이브는 The Powder Toy 홈페이지에 있는 당신의 프로필에 표시되는 평균 점수에 영향을 줄 것입니다. 사람들이 당신의 작품을 보고 댓글을 달고 좋아요를 보내게 하려면 작품을 공개하십시오.\n"
		"\bt비공개된 세이브\bw는 '날짜순 정렬'에서 보이지 않습니다. 이것은 당신의 평균 점수에 영향을 주지 않습니다. 다만, 세이브 ID를 알고 있는 사람은 이러한 작품을 볼 수 있으므로 아예 공개되지 않는 것은 아닙니다. 당신은 특정한 사람들에게만 이러한 세이브를 보여 주기 위해 세이브 ID를 배포할 수 있습니다.\n"
		"\n"
		"어떤 세이브를 즉시 다시 저장하려면 세이브를 열고 저장 단추의 왼쪽 부분의 \bt'현재 시뮬레이션을 재업로드'\bw 단추를 누르십시오. 작품의 설명이나 공개 상태를 바꾸고 싶다면 오른쪽 부분의 \bt'시뮬레이션 속성 변경'\bw 단추를 누르십시오. 단, 이때 세이브의 이름을 바꿀 경우 이 세이브는 완전히 새로운 세이브로 저장되어 기존의 댓글, 좋아요, 태그가 사라지게 됩니다.\n"
		"공개되지 않은 세이브를 완성한 다음 공개하고 싶거나, 현재 공개된 세이브를 비공개로 바꾸고 싶다면, 세이브를 열고 '시뮬레이션 속성 변경' 단추를 누르고, 공개 상태를 변경하면 됩니다. 세이브 검색 화면의 '내 세이브' 메뉴를 통하여 세이브를 선택한 뒤, 하단의 \bt'비공개로 전환' 또는 '삭제'\bw 단추를 통하여서도 해당 작업을 할 수 있습니다.\n"
		"어떤 세이브가 만들어진 지 일주일이 되지 않았고 빠른 속도로 인기를 얻는다면 자동으로 \bt프론트페이지\bw에 올라가게 됩니다. 공개된 세이브만이 프론트페이지에 등록됩니다. 관리자들도 매우 가끔 세이브를 프론트페이지에 올려놓습니다. 관리자들은 또한 규정을 어기거나 적절하지 않다고 생각하는 세이브를 프론트페이지에서 내릴 수 있습니다.\n"
		"한 번 세이브를 만들었다면 세이브를 다시 저장하는 데에는 제한이 없습니다. 짧은 기간의 \bt세이브 편집 기록\bw이 남으므로, 세이브 검색 화면에서 세이브를 마우스 오른쪽 클릭한 뒤 '편집 기록 보기'를 통하여 편집 기록을 볼 수 있습니다. 이것은 실수로 저장하였거나 과거 버전으로 돌아가고 싶을 때 쓰일 수 있습니다.\n"
		;

	new InformationMessage("게시 안내", info, true);
}

void ServerSaveActivity::ShowRules()
{
	String rules =
		"\bo섹션 S - 커뮤니티 내 활동에 관한 규칙\n"
		"\bw아래 규칙들은 The Powder Toy 커뮤니티에서 활동할 때 반드시 지켜야 할 규칙입니다. 이 규칙들은 스태프들에 의해 집행되며, 이러한 규정을 어기는 경우 다른 회원이 우리 스태프들에게 알릴 것입니다. 제1장은 세이브, 댓글, 포럼, 그리고 기타 커뮤니티의 다른 영역에서의 규칙입니다.\n"
		"\n"
		"\bt1. 최대한 바른 문법을 사용하십시오.\bw 우리 커뮤니티의 공식 언어는 영어지만, 특정 국가나 문화권의 사람들이 모이는 경우에까지 영어를 굳이 쓰진 않아도 됩니다. 영어를 잘 하지 못한다면 번역기를 사용하는 것을 추천드립니다.\n"
		"\bt2. 스팸 행위는 금지입니다.\bw 여기서 '스팸'이 항상 여기의 정의에 맞지는 않을 수도 있지만, 대부분은 어떤 행동이 '스팸'인지 아닌지 명백하게 알아차릴 수 있을 것입니다. 아래는 '스팸'으로 간주하여 글 따위가 숨겨지거나 지워질 수 있는 행위입니다:\n"
		   "- 많은 수의 같은 주제를 가진 스레드를 포럼에 올리는 경우. 게임에 대한 피드백이나 제안은 최대한 모아서 올리십시오.\n"
		   "- 오래된 스레드에 대답하여 다시 스레드 목록의 상위로 올리는 경우. 흔히 'necro'나 'necroing'로 불립니다. 이런 스레드의 주제는 오래되었을 수도 있습니다(버그 수정, 아이디어 등). 이러한 경우, 새로운 응답을 위해서는 새로운 스레드를 생성하는 것을 권장합니다.\n"
		   "- '+1' 등의 짧은 대답을 한 스레드에 올리는 경우. 이런 방법을 통해 지속적으로 스레드를 목록 위로 올릴 경우 다른 대답을 찾기 어려워집니다. 당신이 이 스레드를 지지함을 표현하는 '+1' 단추가 이미 있으니, 대답 기능은 건설적인 피드백에 사용하십시오.\n"
		   "- 지나치게 길거나 의미 없는 댓글. 의미 없이 같은 문자를 반복하거나 너무 적은 내용을 쓰는 것 등이 이에 해당합니다. 단, 다른 언어로 댓글을 쓰는 것은 해당하지 않습니다.\n"
		   "- 특정 서식을 남용하는 경우. 대문자, 굵은 글씨, 기울임꼴은 적절하게 쓰이면 좋지만, 글 전체에 이런 서식을 쓰는 것은 적절하지 않습니다.\n"
		"\bt3. 욕설은 자제하십시오.\bw 욕설이 포함된 댓글이나 세이브는 지워질 수 있습니다. 다른 언어로 욕설을 사용하는 것도 이 규칙에 포함됩니다.\n"
		"\bt4. 성적으로 노골적이거나, 불쾌하거나, 기타 부적절한 것을 업로드하지 마십시오.\bw\n"
		   "- 이것은 성행위, 약물, 인종차별, 극단주의 사상, 또는 다른 집단을 공격하거나 비난하는 등의 모든 행위를 포함하며, 또한 위에서 말한 행위만이 이 규칙의 적용 대상인 것도 아닙니다.\n"
		   "- 이러한 주제를 다른 언어로 말하는 것도 금지되어 있습니다. 이 규칙을 회피하려고 하지 마십시오.\n"
		   "- 이 규칙을 어기는 내용을 포함하는 URL이나 이미지를 올리는 것도 금지되어 있습니다. 이것은 당신의 프로필에 이러한 링크를 올려놓거나 글을 써 놓는 것도 해당합니다.\n"
		"\bt5. The Powder Toy와 직접적으로 관련되지 않은 게임이나 사이트, 기타 등을 광고하지 마십시오.\bw\n"
		   "- 이 규칙은 자신의 게임이나 상품을 경험시키거나 광고하는 것을 막기 위해 있습니다.\n"
		   "- 디스코드 등 인증받지 않았거나 비공식적인 커뮤니티를 광고하는 것 역시 금지되어 있습니다.\n"
		"\bt6. 트롤링은 금지입니다.\bw 일부 규칙과 마찬가지로 트롤링에 대한 정확한 정의는 내릴 수 없습니다. 자주 트롤링하는 회원은 이용이 금지될 가능성이 높으며, 그 기간이 다른 이들보다 길 것입니다.\n"
		"\bt7. 타인을 사칭하지 마십시오.\bw 우리 커뮤니티나 타 인터넷 커뮤니티에서 의도적으로 다른 회원과 별명을 비슷하게 등록하는 것은 금지되어 있습니다.\n"
		"\bt8. 관리자의 결정이나 문제점에 관한 내용은 쓰지 마십시오.\bw 이용 정지나 콘텐츠 삭제에 관한 이의가 있다면 메시지 시스템을 이용해서 관리자에게 연락하십시오. 이 외 관리자의 결정에 관한 내용에 관한 글은 자제하십시오.\n"
		"\bt9. 관리자의 일에 참견하는 것은 삼가십시오.\bw 결정을 하는 것은 관리자입니다. 이용자들은 이용 정지가 되거나 규정 위반에 해당할 수 있는 일을 자제해야 합니다. 만약 어떤 일이 규정을 어길 것 같거나 확실하지 않다면 '신고' 단추를 통해서 알리거나 우리 홈페이지의 메시지 시스템을 통해 물어볼 것을 권장합니다.\n"
		"\bt10. 법을 어기는 것을 묵인하는 것은 금지되어 있습니다.\bw 어떤 나라의 법이 어떻게 적용되는지는 확실치 않으나 몇몇 행위는 흔히 볼 수 있을 것입니다. 아래에는 다음과 같은 행위가 있지만 여기에만 국한되지는 않습니다:\n"
		   "- 소프트웨어, 음악, Bagels 등의 복제\n"
		   "- 계정을 해킹하거나 뺏는 것\n"
		   "- 절도, 사기\n"
		"\bt11. 이용자를 쫓아다니거나 괴롭히지 마십시오.\bw 이것은 최근 몇 년 사이 여러 방법으로 나타나며 커지던 문제로, 일반적으로는 아래의 행위를 포함합니다.\n"
		   "- 이용자의 거주지나 실제 정체를 알아내어 공개하는 것, 일명 '신상 털이'\n"
		   "- 어떤 이용자가 연락하고 싶지 않음에도 불구하고 지속적으로 메시지를 보내는 것\n"
		   "- 세이브에 대량으로 '싫어요'를 누르는 것\n"
		   "- 타인의 콘텐츠(세이브, 포럼 스레드 등)에 무례하거나 불필요한 댓글을 다는 것\n"
		   "- 어떤 이용자를 표적으로 공격하도록 다수의 이용자에게 강요하는 것\n"
		   "- 개인적인 언쟁이나 증오. 이것은 댓글에 분쟁을 유발하거나 혐오성이 짙은 세이브 등을 말합니다.\n"
		   "- 종교적이거나 문화적 이유 등으로 사람을 차별하는 행위.\n"
		"\n"
		"\bo섹션 G - 게임 내 규칙\n"
		"\bw섹션 G는 게임 내 행위에 대해 중점적으로 다룹니다. 섹션 S 역시 게임 내 행위에 적용되지만 아래 규칙은 게임 내 커뮤니티에서의 활동에 대해 더욱 중점적으로 다룹니다.\n"
		"\bt1. 타인의 작품을 가져다 쓰지 마십시오.\bw 이는 단순히 타인의 세이브를 다시 업로드하거나 대부분의 영역에 타인의 작품을 이용하는 것을 포함합니다. 파생 작품은 그 의도가 적절하다면 허용됩니다. 기본적으로 원작자가 따로 자유롭게 사용해도 된다고 말하지 않은 이상, 일반적인 규정에 따라 타인의 작품을 썼다면 원작의 제작자를 반드시 언급해야 합니다. 파생 작품은 창의적 이용과 독창적인 부분의 비율에서 그 가치가 결정됩니다. 도용된 세이브는 비공개되거나 비활성화될 것입니다.\n"
		"\bt2. 자기 작품에 자신이 직접 좋아요를 보내거나, 좋아요를 조작하는 것은 금지되어 있습니다.\bw 좋아요 조작이란 많은 계정을 만들어서 자신이나 타인의 세이브에 좋아요를 보내는 것을 말합니다. 이 규정은 엄격하게 적용되므로, 이의 신청이 거의 받아들여지지 않음을 명심하십시오. 당신과 다른 계정이 같은 세이브에 좋아요를 보내고 있지는 않는지 확인하십시오. 다중 계정은 모두 영구적으로 이용이 정지될 것이며, 주 계정은 일시적으로 이용이 정지되고 영향을 받은 모든 세이브가 삭제될 것입니다.\n"
		"\bt3. 어떤 방법으로도 좋아요를 요구하는 것 또한 옳지 않은 행동입니다.\bw 이러한 행위를 하는 세이브는 수정될 때까지 비공개될 것입니다. 아래는 이 규칙에 해당하는 행위의 예시입니다.\n"
		   "- \"좋아요\" 또는 \"싫어요\" 좋아요를 암시하는 기호들. 초록색 화살표나 좋아요를 할 것을 요구하는 것이 이 규정에 어긋납니다.\n"
		   "- 좋아요를 받기 위한 술책들. 이러한 행위는 이를테면 '100 표를 받으면 더 개선된 것을 내놓겠습니다.'와 같은 총 좋아요 수에 대하여 무언가를 내놓겠다는 것을 말합니다. 우리는 이것을 '좋아요 수 파밍'라고 부르고 이러한 행위는 허용되지 않습니다.\n"
		   "- 세이브 사용에 대한 답례로 좋아요를 요구하거나 기타 이유로 좋아요를 요구하는 것 또한 금지입니다.\n"
		"\bt4. 스팸 행위는 금지입니다.\bw 앞서 말하였듯이, 우리가 스팸으로 여기는 것은 정확한 정의가 없습니다. 아래는 스팸으로 간주할 수 있는 행위의 예시입니다.\n"
		   "- 짧은 시간 이내에 비슷한 세이브를 업로드하거나 재업로드하는 행위. 세이브의 조회수나 좋아요 수를 위해서 시스템을 회피하려고 시도하지 마십시오. 이것은 특별한 의도 없이 의미 없거나 빈 세이브를 업로드하는 것도 포함됩니다. 이러한 세이브는 비공개될 것입니다.\n"
		   "- 텍스트로만 채워진 세이브를 올리는 행위. 이것은 공지 사항일 수도 있고 도움을 요청하는 것일 수도 있습니다. 이러한 텍스트만 채워진 세이브에 대비하여 우리는 포럼과 댓글 기능을 만들어 놓았습니다. 이러한 세이브는 프론트페이지에서 내려갈 것입니다.\n"
		   "- 아트 세이브를 올리는 것은 엄격히 금하지는 않지만, 프론트페이지에서는 내려갈 수 있습니다. 우리는 다양한 물질이 창의적으로 사용된 세이브를 높이 평가합니다. 이러한 요소가 부족한(데코만으로 된 세이브 등) 세이브는 일반적으로 프론트페이지에서 내려갈 것입니다.\n"
		"\bt5. 성적으로 노골적이거나 기타 부적절한 콘텐츠를 올리는 것은 삼가십시오.\bw 이러한 세이브는 삭제되고 이용 정지 처분이 내려질 것입니다.\bw\n"
		   "- 이것은 성행위, 약물, 인종차별, 극단주의적 정책, 또는 다른 집단을 공격하거나 비난하는 등의 모든 행위를 포함하며, 또한 위에서 말한 행위만이 이 규칙의 적용 대상인 것도 아닙니다.\n"
		   "- 이 규칙을 회피하려고 하지 마십시오. 고의로 이러한 개념이나 발상을 나타내는 것은 방법이 직접적이나 간접적이냐에 관계없이 규정을 어기는 행위입니다.\n"
		   "- 다른 언어로 이러한 주제를 표현하는 것도 금지되어 있습니다. 어떤 방법으로든 이 규칙을 회피하려고 하지 마십시오.\n"
		   "- 이 규칙을 어기는 내용을 포함하는 URL이나 이미지를 올리는 것도 금지되어 있습니다. 이것은 당신의 프로필에 이러한 링크를 올려놓거나 글을 써 놓는 것도 해당합니다.\n"
		"\bt6. 이미지 변환 프로그램 등은 엄격하게 금지되어 있습니다.\bw 이것은 그림 등을 따 오거나 세이브를 만들어 주는 모든 종류의 스크립트나 비공식적인 도구들을 포함합니다. CGI를 사용한 세이브는 삭제되며 이용 정지 처분을 받게 될 것입니다.\n"
		"\bt7. 로고나 표지판은 최소한으로 사용하십시오.\bw 이러한 세이브는 프론트페이지에서 삭제될 것입니다. 아래는 이 규칙이 제한하는 것입니다:\n"
		   "- 로고 남용\n"
		   "- 특별한 의도 없는 표지판의 사용\n"
		   "- 가짜 업데이트 또는 알림 표지판\n"
		   "- 관련이 없는 세이브로 링크하는 행위\n"
		"\bt8. 주제에 어긋나거나 부적절한 태그를 사용하지 마십시오.\bw 태그는 검색 기능을 개선하기 위해 존재하는 기능입니다. 태그들은 일반적으로 세이브를 설명하는 1개의 단어로만 이루어집니다. 문장형 태그 또는 주관적인 태그는 삭제될 것입니다. 부적절하거나 모욕적인 태그를 올리는 것은 이용 정지 처분의 대상이 될 수 있는 행위입니다.\n"
		"\bt9. 고의로 랙이나 크래시를 유발하는 세이브는 금지되어 있습니다. \bw 다수의 이용자들이 크래시나 랙을 유발한다고 신고하는 세이브가 이 규정에 어긋나는 세이브가 됩니다. 이러한 세이브는 프론트페이지에서 내려가거나 비활성화될 것입니다.\n"
		"\bt10. 신고 시스템을 남용하지 마십시오.\bw '나쁜 세이브' 등의 이유나 의미 없는 내용으로 신고하면 우리의 시간이 의미 없이 쓰이게 됩니다. 규정 위반인 것 같거나 커뮤니티에서 사건·사고가 일어나지 않는다면 신고하지 마십시오. 세이브의 내용이 규칙을 위반하거나 커뮤니티에 문제를 일으키는 것 같다면 신고하십시오! 여러분이 좋은 의도로 신고한다면 여러분이 이용 정지될 일은 절대 없습니다.\n"
		"\bt11. 어떤 세이브를 프론트페이지에서 내려달라고 하는 행위는 삼가십시오.\bw 세이브가 규칙을 어기지 않는 이상, 그 세이브는 프론트페이지에 머무를 것입니다. 이 규정은 아트 세이브에도 예외가 아니므로, 아트 세이브에도 이러한 요구는 하지 마십시오.\n"
		"\n"
		"\bo섹션 R: 기타\n"
		"\bw관리자들은 그들의 판단에 따라서 규칙을 해석할 수 있습니다. 이 규칙이 항상 동등하게 적용되진 않고, 어떤 규칙은 조금 덜 엄격하게 시행됩니다. 규칙에 맞는지 안 맞는지 최종적인 판단은 관리자들이 하지만, 불쾌한 모든 행동을 다루기 위해 우리는 항상 최선을 다해 왔습니다. 만약 규칙이 변경된다면 이 스레드에 공지 사항이 올라올 것입니다.\n"
		"\n"
		"규칙을 어기면 포럼 게시글이나 댓글의 삭제, 세이브의 비공개나 비활성화, 프론트페이지에서 삭제, 혹은 더 나아가서 일시적이거나 영구적인 이용 정지 처분을 받게 됩니다. 이런 규칙들을 시행하기 위해 자동적이거나 수동적인 여러 방법을 적용하고 있습니다. 처분 수위나 결정은 관리자들 간에 일관되지 않을 수 있습니다.\n"
		"\n"
		"어떤 행위가 규칙에 맞는지 어긋나는지 묻고 싶다면 언제나 관리자에게 연락하십시오.";

	new InformationMessage("세이브 업로드 규칙", rules, true);
}

void ServerSaveActivity::CheckName(String newname)
{
	if (newname.length() && newname == save->GetName() && save->GetUserName() == Client::Ref().GetAuthUser().Username)
		titleLabel->SetText("시뮬레이션 속성 수정:");
	else
		titleLabel->SetText("새 시뮬레이션 업로드:");
}

void ServerSaveActivity::OnTick(float dt)
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

	if (uploadSaveRequest && uploadSaveRequest->CheckDone())
	{
		okayButton->Enabled = true;
		try
		{
			save->SetID(uploadSaveRequest->Finish());
			Exit();
			new SaveIDMessage(save->GetID());
			if (onUploaded)
			{
				onUploaded(std::move(save));
			}
		}
		catch (const http::RequestError &ex)
		{
			new ErrorMessage("오류", "업로드에 실패하였습니다. 오류:\n" + ByteString(ex.what()).FromUtf8());
		}
		uploadSaveRequest.reset();
	}

	if(saveUploadTask)
		saveUploadTask->Poll();
}

void ServerSaveActivity::OnDraw()
{
	Graphics * g = GetGraphics();
	g->BlendRGBAImage(saveToServerImage->data(), RectSized(Vec2(-36, 0), saveToServerImage->Size()));
	g->DrawFilledRect(RectSized(Position, Size).Inset(-1), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);

	if (Size.X > 220)
		g->DrawLine(Position + Vec2(Size.X / 2 - 1, 0), Position + Vec2(Size.X / 2 - 1, Size.Y - 1), 0xFFFFFF_rgb);

	if (thumbnail)
	{
		auto rect = RectSized(Position + Vec2(Size.X / 2 + (Size.X / 2 - thumbnail->Size().X) / 2, 25), thumbnail->Size());
		g->BlendImage(thumbnail->Data(), 0xFF, rect);
		g->DrawRect(rect, 0xB4B4B4_rgb);
	}
}

ServerSaveActivity::~ServerSaveActivity()
{
	if (thumbnailRenderer)
	{
		thumbnailRenderer->Abandon();
	}
	delete saveUploadTask;
}
