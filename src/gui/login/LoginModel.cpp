#include "LoginModel.h"

#include "LoginView.h"

#include "client/Client.h"

LoginModel::LoginModel():
	currentUser(0, "")
{

}

void LoginModel::Login(ByteString username, ByteString password)
{
	if (username.Contains("@"))
	{
		statusText = "이메일이 아닌 The Powder Toy 계정으로 로그인하세요. 계정이 없다면 https://powdertoy.co.uk/Register.html에서 새로 만들 수";
		loginStatus = false;
		notifyStatusChanged();
		return;
	}
	statusText = "로그인하는 중...";
	loginStatus = false;
	notifyStatusChanged();
	LoginStatus status = Client::Ref().Login(username, password, currentUser);
	switch(status)
	{
	case LoginOkay:
		statusText = "로그인됨";
		loginStatus = true;
		break;
	case LoginError:
		statusText = Client::Ref().GetLastError();
		break;
	}
	notifyStatusChanged();
}

void LoginModel::AddObserver(LoginView * observer)
{
	observers.push_back(observer);
}

String LoginModel::GetStatusText()
{
	return statusText;
}

User LoginModel::GetUser()
{
	return currentUser;
}

bool LoginModel::GetStatus()
{
	return loginStatus;
}

void LoginModel::notifyStatusChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyStatusChanged(this);
	}
}

LoginModel::~LoginModel() {
}

