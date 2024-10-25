#ifndef MYFRAME_H
#define MYFRAME_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <string>
#include <vector>
#include <mutex>

struct ClientInfo {
    std::string username;
    std::string hostname;
    std::string ip;

    ClientInfo(const std::string& user, const std::string& host, const std::string& ipAddr)
        : username(user), hostname(host), ip(ipAddr) {}
};

class MyFrame : public wxFrame {
public:
    MyFrame();
    void StartServer();
    void AddClient(const ClientInfo& client);
    void OnScreenButton(wxCommandEvent& event);
    void TakeScreenshot();

private:
    wxListCtrl* clientList;
    std::vector<ClientInfo> clients;
    std::mutex clientMutex;

    wxDECLARE_EVENT_TABLE();
};

#endif // MYFRAME_H
