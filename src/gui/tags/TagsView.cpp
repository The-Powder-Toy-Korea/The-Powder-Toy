#include "TagsView.h"

#include "TagsController.h"
#include "TagsModel.h"
#include "TagsModelException.h"

#include "client/Client.h"
#include "client/SaveInfo.h"

#include "graphics/Graphics.h"

#include "gui/dialogues/ErrorMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Label.h"

#include <SDL.h>

TagsView::TagsView():
	ui::Window(ui::Point(-1, -1), ui::Point(195, 250))
{
	closeButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(195, 16), "닫기");
	closeButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	closeButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	closeButton->SetActionCallback({ [this] { c->Exit(); } });
	AddComponent(closeButton);
	SetCancelButton(closeButton);

	tagInput = new ui::Textbox(ui::Point(8, Size.Y-40), ui::Point(Size.X-65, 16), "", "새 태그 이름");
	tagInput->Appearance.icon = IconTag;
	tagInput->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tagInput->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	tagInput->SetLimit(16);
	AddComponent(tagInput);
	FocusComponent(tagInput);

	addButton = new ui::Button(ui::Point(tagInput->Position.X+tagInput->Size.X+4, tagInput->Position.Y), ui::Point(45, 16), "추가");
	addButton->Appearance.icon = IconAdd;
	addButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	addButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	addButton->SetActionCallback({ [this] { addTag(); } });
	AddComponent(addButton);

	if (!Client::Ref().GetAuthUser().UserID)
		addButton->Enabled = false;

	title = new ui::Label(ui::Point(5, 5), ui::Point(185, 28), "태그 관리: \bg태그는 검색 결과를 개선하기 위해서만 쓰여야 합니다.");
	title->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	title->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	title->SetMultiline(true);
	AddComponent(title);
}

void TagsView::OnTick()
{
	c->Tick();
}

void TagsView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);
}

void TagsView::NotifyTagsChanged(TagsModel * sender)
{
	for (size_t i = 0; i < tags.size(); i++)
	{
		RemoveComponent(tags[i]);
		delete tags[i];
	}
	tags.clear();
	
	if(sender->GetSave())
	{
		std::list<ByteString> Tags = sender->GetSave()->GetTags();
		int i = 0;
		for (auto &tag : Tags)
		{
			ui::Label * tempLabel = new ui::Label(ui::Point(35, 35+(16*i)), ui::Point(120, 16), tag.FromUtf8());
			tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;			tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
			tags.push_back(tempLabel);
			AddComponent(tempLabel);

			if(sender->GetSave()->GetUserName() == Client::Ref().GetAuthUser().Username || Client::Ref().GetAuthUser().UserElevation == User::ElevationAdmin || Client::Ref().GetAuthUser().UserElevation == User::ElevationMod)
			{
				ui::Button * tempButton = new ui::Button(ui::Point(15, 37+(16*i)), ui::Point(11, 12));
				tempButton->Appearance.icon = IconDelete;
				tempButton->Appearance.Border = ui::Border(0);
				tempButton->Appearance.Margin.Top += 2;
				tempButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
				tempButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
				tempButton->SetActionCallback({ [this, tag] {
					c->RemoveTag(tag);
				} });
				tags.push_back(tempButton);
				AddComponent(tempButton);
			}
			i++;
		}
	}
}

void TagsView::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	switch(key)
	{
	case SDLK_KP_ENTER:
	case SDLK_RETURN:
		if(IsFocused(tagInput))
		{
			addTag();
		}
		break;
	}
}

void TagsView::addTag()
{
	if (tagInput->GetText().length() < 4)
	{
		new ErrorMessage("태그가 너무 짧음", "네 글자 이상의 태그를 입력해야 합니다.");
		return;
	}
	c->AddTag(tagInput->GetText().ToUtf8());
	tagInput->SetText("");
}
