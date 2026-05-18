#ifndef GUI_H
#define GUI_H

#include <windows.h>
#include <gdiplus.h>
#include <string>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
#include "LinkedList.h"
#include "Models.h"
#include "HashTable.h"
#include "BST.h"
#include "Queue.h"
#include "StorageManager.h"

enum GUIState { STATE_LOGIN, STATE_REGISTER, STATE_DASHBOARD };

#define SIDEBAR_W    200
#define LOGIN_HALF_W 525
#define SIDEBAR_CLR  RGB(17, 24, 39)
#define HEADER_CLR   RGB(28, 124, 84)
#define BG_CLR       RGB(244, 247, 251)
#define CARD_H       100
#define ACCENT_CLR   RGB(18, 151, 111)
#define GREEN_CLR    RGB(18, 151, 111)
#define ORANGE_CLR   RGB(245, 158, 11)
#define RED_CLR      RGB(239, 68, 68)
#define COMMISSION_RATE 0.05

class GUI {
private:
    HINSTANCE hInstance;
    HWND hwndMain;

    // Login
    HWND hLoginTitle, hLoginSubtitle, hUserLabel, hPassLabel;
    HWND hUserEdit, hPassEdit, hBtnLogin, hBtnGoToRegister;
    HBITMAP hLogoImage;
    HWND hLogoControl;

    // Register
    HWND hRegTitle, hRegSubtitle, hRegUserLabel, hRegPassLabel, hRegRoleLabel;
    HWND hRegUserEdit, hRegPassEdit, hRegRoleCombo;
    HWND hBtnRegister, hBtnBackToLogin;

    // Dashboard shared
    HWND hDashTitle, hUserWelcome;
    HWND hwndList;
    HWND hBtnLogout, hBtnSort;
    HWND hSearchEdit, hBtnSearch, hBtnResetSearch;
    HWND hReportTitle, hReportBox;

    // Seller/Admin add form
    HWND hTypeCombo, hPrice, hLocation, hParam1, hParam2;
    HWND hLblType, hLblPrice, hLblLoc, hLblP1, hLblP2, hBtnAdd;

    // Role-specific buttons
    HWND hBtnBuy, hBtnApprove, hBtnDelete, hBtnProcess;

    // Document upload (Buyer/Seller)
    HWND hBtnUploadDoc, hDocStatusLabel;

    // Admin verification panel
    HWND hVerifyTitle, hVerifyList;
    HWND hBtnOpenDoc, hBtnApproveAcc, hBtnRejectAcc;

    // Car specific uploads
    HWND hBtnCarImg1, hBtnCarImg2, hBtnOwnerDoc, hBtnBodyDoc;
    HWND hLblImg1, hLblImg2, hLblOwner, hLblBody;
    
    // Buyer actions
    HWND hBtnDetails, hBtnPay;

    // Jiji-style Details Panel
    HWND hDetailPanel, hMainImgCtrl, hTitleLbl, hPriceLbl, hContactBtn;
    ULONG_PTR gdiplusToken;
    Listing* selectedListing;

    // Commission tracking
    double totalCommission;

    // Search/filter state
    std::string activeSearch;

    // Data
    LinkedList<Listing*> listings;
    LinkedList<Account*> accounts;
    Queue<Inquiry*> orderQueue;
    HashTable hashTable;
    BST bst;
    StorageManager* storage;

    Account* currentUser;
    GUIState currentState;

    HFONT hFontTitle, hFontBold, hFontNormal, hFontSmall;
    HBRUSH hBgBrush, hSidebarBrush, hHeaderBrush;
    int hoveredSideBtn;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void showLogin();
    void showRegister();
    void showDashboard();
    void hideAll();

    void attemptLogin();
    void attemptRegister();
    void logout();

    void populateList();
    void populateVerifyList();
    void applySearch();
    void clearSearch();
    bool ensureCurrentUserCanTrade(const char* action);
    int generateNextListingId();
    void addListing();
    void deleteSelectedListing();
    void buyListing();
    void approveListing();
    void processOrder();
    void uploadDocument();
    void openSelectedDocument();
    void approveSelectedAccount();
    void rejectSelectedAccount();
    void viewListingDetails();
    void updateDetailPanel(Listing* item);
    void pay();
    void uploadCarFile(int id);

    void drawSidebarButton(LPDRAWITEMSTRUCT dis, const char* label);
    void drawCustomButton(LPDRAWITEMSTRUCT dis, const char* label, COLORREF color);
    void paintBackground(HWND hwnd, HDC hdc);

public:
    GUI(HINSTANCE hInst, StorageManager* st);
    ~GUI();
    int run();
};

#endif
