#include "GUI.h"
#include "Algorithms.h"
#include <string>
#include <cstdlib>
#include <cctype>
#include <commdlg.h>
#include <shellapi.h>

#define ID_BTN_LOGIN 201
#define ID_BTN_LOGOUT 202
#define ID_BTN_ADD 203
#define ID_BTN_SORT 204
#define ID_BTN_DELETE 205
#define ID_LISTBOX 206
#define ID_COMBO_TYPE 207
#define ID_BTN_GOTO_REG 208
#define ID_BTN_REGISTER 209
#define ID_BTN_BACK_LOGIN 210
#define ID_COMBO_ROLE 211
#define ID_BTN_BUY 212
#define ID_BTN_APPROVE 213
#define ID_BTN_PROCESS 214
#define ID_BTN_UPLOAD 215
#define ID_VERIFY_LIST 216
#define ID_BTN_OPEN_DOC 217
#define ID_BTN_APPROVE_ACC 218
#define ID_BTN_REJECT_ACC 219
#define ID_BTN_CAR_IMG1 220
#define ID_BTN_CAR_IMG2 221
#define ID_BTN_OWNER_DOC 222
#define ID_BTN_BODY_DOC 223
#define ID_BTN_DETAILS 224
#define ID_BTN_PAY 225
#define ID_BTN_SEARCH 226
#define ID_BTN_RESET_SEARCH 227

GUI* g_GUI = NULL;
static string g_pendingDocPath = ""; 
static string g_pendingImg1 = "";
static string g_pendingImg2 = "";
static string g_pendingOwnerDoc = "";
static string g_pendingBodyDoc = "";

static string normalizeSearchText(string value) {
    for (int i = 0; i < (int)value.size(); i++) {
        if (value[i] >= 'A' && value[i] <= 'Z') value[i] = (char)(value[i] - 'A' + 'a');
    }
    return value;
}

static bool isBlankText(const string& value) {
    for (int i = 0; i < (int)value.size(); i++) {
        if (!isspace((unsigned char)value[i])) return false;
    }
    return true;
}

static bool parsePositiveDouble(const char* text, double& value) {
    if (!text) return false;
    char* endPtr = NULL;
    value = strtod(text, &endPtr);
    if (endPtr == text) return false;
    while (*endPtr && isspace((unsigned char)*endPtr)) endPtr++;
    return *endPtr == '\0' && value > 0;
}

static bool parsePositiveInt(const char* text, int& value) {
    if (!text) return false;
    char* endPtr = NULL;
    long parsed = strtol(text, &endPtr, 10);
    if (endPtr == text) return false;
    while (*endPtr && isspace((unsigned char)*endPtr)) endPtr++;
    if (*endPtr != '\0' || parsed <= 0 || parsed > 2147483647L) return false;
    value = (int)parsed;
    return true;
}

static string hashPassword(const string& password) {
    unsigned long hash = 5381;
    for (int i = 0; i < (int)password.size(); i++) {
        hash = ((hash << 5) + hash) + (unsigned char)password[i];
    }
    return "h$" + to_string_compat(hash);
}

static bool isHashedPassword(const string& storedPassword) {
    return storedPassword.size() > 2 && storedPassword[0] == 'h' && storedPassword[1] == '$';
}

static bool passwordMatches(const string& storedPassword, const string& inputPassword) {
    if(isHashedPassword(storedPassword)) return storedPassword == hashPassword(inputPassword);
    return storedPassword == inputPassword;
}

static void drawRoundBox(HDC dc, RECT r, COLORREF fill, COLORREF border, int radius) {
    HBRUSH br=CreateSolidBrush(fill);
    HPEN pen=CreatePen(PS_SOLID,1,border);
    HBRUSH oldB=(HBRUSH)SelectObject(dc,br);
    HPEN oldP=(HPEN)SelectObject(dc,pen);
    RoundRect(dc,r.left,r.top,r.right,r.bottom,radius,radius);
    SelectObject(dc,oldB); SelectObject(dc,oldP);
    DeleteObject(br); DeleteObject(pen);
}

static void drawSoftShadow(HDC dc, RECT r) {
    RECT s={r.left+2,r.top+3,r.right+2,r.bottom+3};
    drawRoundBox(dc,s,RGB(226,232,240),RGB(226,232,240),14);
}

static void drawPanel(HDC dc, RECT r) {
    drawSoftShadow(dc,r);
    drawRoundBox(dc,r,RGB(255,255,255),RGB(226,232,240),14);
}

GUI::GUI(HINSTANCE hInst, StorageManager* st)
    : hInstance(hInst), storage(st), currentUser(NULL),
      currentState(STATE_LOGIN), hoveredSideBtn(-1), totalCommission(0.0),
      activeSearch(""), selectedListing(NULL) {
    g_GUI = this;
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    if(storage){ storage->loadListings(listings); storage->loadAccounts(accounts); storage->loadOrders(orderQueue); }
    for(int i=0;i<listings.getSize();i++){
        Listing* it=*listings.get(i);
        hashTable.insert(it->id,it);
        bst.insert(it);
        if(it->status==L_STATUS_SOLD || it->status==L_STATUS_RENTED) totalCommission += it->price * COMMISSION_RATE;
    }

    hFontTitle  = CreateFont(34,0,0,0,FW_BOLD,  FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Segoe UI");
    hFontBold   = CreateFont(19,0,0,0,FW_BOLD,  FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Segoe UI");
    hFontNormal = CreateFont(16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Segoe UI");
    hFontSmall  = CreateFont(13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Segoe UI");

    hBgBrush      = CreateSolidBrush(BG_CLR);
    hSidebarBrush = CreateSolidBrush(SIDEBAR_CLR);
    hHeaderBrush  = CreateSolidBrush(HEADER_CLR);
    hLogoImage    = (HBITMAP)LoadImage(NULL,"logo.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
}

GUI::~GUI(){
    GdiplusShutdown(gdiplusToken);
    if(storage){ storage->saveListings(listings); storage->saveOrders(orderQueue); }
    for(int i=0;i<listings.getSize();i++) delete *listings.get(i);
    for(int i=0;i<accounts.getSize();i++) delete *accounts.get(i);
    while(!orderQueue.isEmpty()) delete orderQueue.dequeue();
    DeleteObject(hFontTitle); DeleteObject(hFontBold); DeleteObject(hFontNormal); DeleteObject(hFontSmall);
    DeleteObject(hBgBrush); DeleteObject(hSidebarBrush); DeleteObject(hHeaderBrush);
    if(hLogoImage) DeleteObject(hLogoImage);
}

int GUI::run(){
    WNDCLASS wc={0};
    wc.lpfnWndProc=WindowProc; wc.hInstance=hInstance; wc.lpszClassName="BrokerApp";
    wc.hbrBackground=hBgBrush; wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    RegisterClass(&wc);
    hwndMain=CreateWindowEx(0,"BrokerApp","Online Broker Management System",
        WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1050,680,NULL,NULL,hInstance,NULL);
    if(!hwndMain) return 0;
    ShowWindow(hwndMain,SW_SHOW); UpdateWindow(hwndMain);
    MSG msg={0};
    while(GetMessage(&msg,NULL,0,0)){ TranslateMessage(&msg); DispatchMessage(&msg); }
    return 0;
}

void GUI::paintBackground(HWND hwnd, HDC hdc){
    RECT rc; GetClientRect(hwnd,&rc);
    RECT sidebar={0,0,SIDEBAR_W,rc.bottom}; FillRect(hdc,&sidebar,hSidebarBrush);
    RECT header={SIDEBAR_W,0,rc.right,64};  FillRect(hdc,&header,hHeaderBrush);
    RECT content={SIDEBAR_W,60,rc.right,rc.bottom}; FillRect(hdc,&content,hBgBrush);

    RECT headerGlow={SIDEBAR_W,60,rc.right,64};
    HBRUSH glow=CreateSolidBrush(RGB(206, 238, 224)); FillRect(hdc,&headerGlow,glow); DeleteObject(glow);

    RECT listPanel={SIDEBAR_W+14,82,SIDEBAR_W+506,448};
    RECT rightPanel={SIDEBAR_W+512,82,rc.right-18,rc.bottom-26};
    RECT verifyPanel={SIDEBAR_W+14,456,SIDEBAR_W+506,rc.bottom-26};
    drawPanel(hdc,listPanel);
    drawPanel(hdc,rightPanel);
    if(currentUser && currentUser->role==ROLE_ADMIN) drawPanel(hdc,verifyPanel);

    SetBkMode(hdc,TRANSPARENT); SetTextColor(hdc,RGB(248, 250, 252));
    SelectObject(hdc,hFontBold); RECT pr={20,15,SIDEBAR_W,45};
    DrawText(hdc,"Broker",-1,&pr,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    SelectObject(hdc,hFontSmall); SetTextColor(hdc,RGB(180, 199, 214));
    RECT sr={20,42,SIDEBAR_W-18,64};
    DrawText(hdc,"Management Suite",-1,&sr,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
}

void GUI::drawSidebarButton(LPDRAWITEMSTRUCT dis, const char* label){
    HDC dc=dis->hDC; RECT r=dis->rcItem;
    bool hot=(dis->itemState&ODS_SELECTED);
    HBRUSH base=CreateSolidBrush(SIDEBAR_CLR);
    FillRect(dc,&r,base); DeleteObject(base);
    RECT pill={r.left+12,r.top+6,r.right-12,r.bottom-6};
    if(hot){
        drawRoundBox(dc,pill,RGB(31, 41, 55),RGB(52, 211, 153),12);
        RECT strip={pill.left,pill.top+7,pill.left+4,pill.bottom-7};
        HBRUSH ab=CreateSolidBrush(RGB(52, 211, 153)); FillRect(dc,&strip,ab); DeleteObject(ab);
    }
    SetBkMode(dc,TRANSPARENT); SetTextColor(dc,hot?RGB(255,255,255):RGB(203, 213, 225));
    SelectObject(dc,g_GUI->hFontNormal);
    RECT tr={r.left+28,r.top,r.right-12,r.bottom};
    DrawText(dc,label,-1,&tr,DT_SINGLELINE|DT_VCENTER|DT_LEFT);
}

void GUI::drawCustomButton(LPDRAWITEMSTRUCT dis, const char* label, COLORREF color){
    HDC dc=dis->hDC; RECT r=dis->rcItem;
    bool hot=(dis->itemState&ODS_SELECTED);
    bool filled=(color==ACCENT_CLR || color==GREEN_CLR || color==RED_CLR);
    COLORREF hotFill=color;
    if(color==ACCENT_CLR || color==GREEN_CLR) hotFill=RGB(13, 128, 96);
    else if(color==RED_CLR) hotFill=RGB(220, 38, 38);
    COLORREF fill=filled ? (hot?hotFill:color) : (hot?RGB(241, 245, 249):RGB(255, 255, 255));
    COLORREF border=filled ? fill : (hot?color:RGB(203, 213, 225));
    drawRoundBox(dc,r,fill,border,12);

    SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc, filled?RGB(255,255,255):(hot?color:RGB(51, 65, 85)));
    SelectObject(dc,filled?g_GUI->hFontBold:g_GUI->hFontNormal);
    DrawText(dc,label,-1,&r,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
}

void GUI::hideAll(){
    HWND all[]={
        hLogoControl,hLoginTitle,hLoginSubtitle,hUserLabel,hPassLabel,hUserEdit,hPassEdit,hBtnLogin,hBtnGoToRegister,
        hRegTitle,hRegSubtitle,hRegUserLabel,hRegPassLabel,hRegRoleLabel,hRegUserEdit,hRegPassEdit,hRegRoleCombo,hBtnRegister,hBtnBackToLogin,
        hDashTitle,hUserWelcome,hwndList,hSearchEdit,hBtnSearch,hBtnResetSearch,
        hTypeCombo,hPrice,hLocation,hParam1,hParam2,
        hLblType,hLblPrice,hLblLoc,hLblP1,hLblP2,hBtnAdd,
        hBtnLogout,hBtnSort,hBtnDelete,hBtnBuy,hBtnApprove,hBtnProcess,
        hReportTitle,hReportBox,
        hBtnUploadDoc,hDocStatusLabel,
        hVerifyTitle,hVerifyList,hBtnOpenDoc,hBtnApproveAcc,hBtnRejectAcc,
        hLblImg1,hBtnCarImg1,hLblImg2,hBtnCarImg2,hLblOwner,hBtnOwnerDoc,hLblBody,hBtnBodyDoc,
        hBtnDetails,hBtnPay,
        hDetailPanel, hMainImgCtrl, hTitleLbl, hPriceLbl, hContactBtn
    };
    for(int i=0;i<(int)(sizeof(all)/sizeof(HWND));i++) if(all[i]) ShowWindow(all[i],SW_HIDE);
}

void GUI::showLogin(){
    hideAll();
    SetWindowPos(hLogoControl, NULL, 0, 0, LOGIN_HALF_W, 680, SWP_NOZORDER);
    ShowWindow(hLogoControl,SW_SHOW);
    ShowWindow(hLoginTitle,SW_SHOW); ShowWindow(hLoginSubtitle,SW_SHOW);
    ShowWindow(hUserLabel,SW_SHOW); ShowWindow(hUserEdit,SW_SHOW);
    ShowWindow(hPassLabel,SW_SHOW); ShowWindow(hPassEdit,SW_SHOW);
    ShowWindow(hBtnLogin,SW_SHOW); ShowWindow(hBtnGoToRegister,SW_SHOW);
    SetWindowText(hUserEdit,""); SetWindowText(hPassEdit,"");
}

void GUI::showRegister(){
    hideAll();
    g_pendingDocPath = "";
    int LX = LOGIN_HALF_W + 80;
    SetWindowPos(hLogoControl, NULL, 0, 0, LOGIN_HALF_W, 680, SWP_NOZORDER);
    SetWindowPos(hDocStatusLabel, NULL, LX+30, 335, 300, 18, SWP_NOZORDER);
    SetWindowPos(hBtnUploadDoc, NULL, LX+30, 355, 300, 32, SWP_NOZORDER);
    ShowWindow(hLogoControl,SW_SHOW);
    ShowWindow(hRegTitle,SW_SHOW); ShowWindow(hRegSubtitle,SW_SHOW);
    ShowWindow(hRegUserLabel,SW_SHOW); ShowWindow(hRegUserEdit,SW_SHOW);
    ShowWindow(hRegPassLabel,SW_SHOW); ShowWindow(hRegPassEdit,SW_SHOW);
    ShowWindow(hRegRoleLabel,SW_SHOW); ShowWindow(hRegRoleCombo,SW_SHOW);
    ShowWindow(hDocStatusLabel,SW_SHOW); ShowWindow(hBtnUploadDoc,SW_SHOW);
    ShowWindow(hBtnRegister,SW_SHOW); ShowWindow(hBtnBackToLogin,SW_SHOW);
    SetWindowText(hDocStatusLabel,"Attach your ID document");
    SetWindowText(hRegUserEdit,""); SetWindowText(hRegPassEdit,"");
    SendMessage(hRegRoleCombo,CB_SETCURSEL,0,0);
}
void GUI::showDashboard(){
    hideAll();
    SetWindowPos(hLogoControl, NULL, 0, 0, SIDEBAR_W, 680, SWP_NOZORDER);
    string roleStr=(currentUser->role==ROLE_ADMIN)?"Admin":(currentUser->role==ROLE_SELLER)?"Seller":"Buyer";

    // Update welcome text with role and verification status
    string statStr = (currentUser->status==ACC_VERIFIED) ? " [VERIFIED]" : (currentUser->status==ACC_REJECTED) ? " [REJECTED]" : " [PENDING]";
    if(currentUser->role==ROLE_ADMIN) statStr = ""; // admin doesn't need status badge
    
    string welcome="  "+currentUser->username+"   ["+roleStr+"]" + statStr;
    SetWindowText(hUserWelcome,welcome.c_str());
    
    ShowWindow(hDashTitle,SW_SHOW); ShowWindow(hUserWelcome,SW_SHOW);
    ShowWindow(hSearchEdit,SW_SHOW); ShowWindow(hBtnSearch,SW_SHOW); ShowWindow(hBtnResetSearch,SW_SHOW);
    ShowWindow(hwndList,SW_SHOW);
    ShowWindow(hBtnLogout,SW_SHOW); ShowWindow(hBtnSort,SW_SHOW);
    ShowWindow(hReportTitle,SW_SHOW); ShowWindow(hReportBox,SW_SHOW);

    if(currentUser->role==ROLE_ADMIN || (currentUser->role==ROLE_SELLER && currentUser->status==ACC_VERIFIED)){
        ShowWindow(hLblType,SW_SHOW); ShowWindow(hTypeCombo,SW_SHOW);
        ShowWindow(hLblPrice,SW_SHOW); ShowWindow(hPrice,SW_SHOW);
        ShowWindow(hLblLoc,SW_SHOW); ShowWindow(hLocation,SW_SHOW);
        ShowWindow(hLblP1,SW_SHOW); ShowWindow(hParam1,SW_SHOW);
        ShowWindow(hLblP2,SW_SHOW); ShowWindow(hParam2,SW_SHOW);
        ShowWindow(hBtnAdd,SW_SHOW);
        
        int type = SendMessage(hTypeCombo, CB_GETCURSEL, 0, 0);
        int RX = SIDEBAR_W + 520;
        
        if(type == 1){ // CAR
            SetWindowText(hLblP1,"Model");
            SetWindowText(hLblP2,"Condition");
            ShowWindow(hLblImg1, SW_SHOW); ShowWindow(hBtnCarImg1, SW_SHOW);
            ShowWindow(hLblImg2, SW_SHOW); ShowWindow(hBtnCarImg2, SW_SHOW);
            ShowWindow(hLblOwner, SW_SHOW); ShowWindow(hBtnOwnerDoc, SW_SHOW);
            ShowWindow(hLblBody, SW_SHOW); ShowWindow(hBtnBodyDoc, SW_SHOW);
            
            // Move Add button and Stats down
            SetWindowPos(hBtnAdd, NULL, RX, 495, 240, 38, SWP_NOZORDER);
            SetWindowPos(hReportTitle, NULL, RX, 545, 240, 22, SWP_NOZORDER);
            SetWindowPos(hReportBox, NULL, RX, 570, 240, 85, SWP_NOZORDER);
        } else { // HOUSE
            SetWindowText(hLblP1,"Bedrooms");
            SetWindowText(hLblP2,"Size (sqm)");
            ShowWindow(hLblImg1, SW_HIDE); ShowWindow(hBtnCarImg1, SW_HIDE);
            ShowWindow(hLblImg2, SW_HIDE); ShowWindow(hBtnCarImg2, SW_HIDE);
            ShowWindow(hLblOwner, SW_HIDE); ShowWindow(hBtnOwnerDoc, SW_HIDE);
            ShowWindow(hLblBody, SW_HIDE); ShowWindow(hBtnBodyDoc, SW_HIDE);
            
            // Move Add button and Stats up
            SetWindowPos(hBtnAdd, NULL, RX, 385, 240, 38, SWP_NOZORDER);
            SetWindowPos(hReportTitle, NULL, RX, 440, 240, 22, SWP_NOZORDER);
            SetWindowPos(hReportBox, NULL, RX, 465, 240, 130, SWP_NOZORDER);
        }
        ShowWindow(hReportTitle, SW_SHOW); ShowWindow(hReportBox, SW_SHOW);
    }
    if(currentUser->role==ROLE_ADMIN){
        ShowWindow(hBtnDelete,SW_SHOW); ShowWindow(hBtnApprove,SW_SHOW); ShowWindow(hBtnProcess,SW_SHOW);
        ShowWindow(hVerifyTitle,SW_SHOW); ShowWindow(hVerifyList,SW_SHOW);
        ShowWindow(hBtnOpenDoc,SW_SHOW); ShowWindow(hBtnApproveAcc,SW_SHOW); ShowWindow(hBtnRejectAcc,SW_SHOW);
    }
    if(currentUser->role==ROLE_BUYER) {
        ShowWindow(hBtnBuy,SW_SHOW);
        // Detail panel controls are shown only when an item is selected
        selectedListing = NULL;
    }

    // Unverified buyers/sellers can upload a new ID document directly from the dashboard.
    if(currentUser->role!=ROLE_ADMIN && currentUser->status!=ACC_VERIFIED){
        string docMsg = (currentUser->status==ACC_REJECTED)
            ? "Verification rejected. Upload a new ID document."
            : "Account pending verification. You can update your ID document.";
        SetWindowText(hDocStatusLabel, docMsg.c_str());
        SetWindowPos(hDocStatusLabel, NULL, SIDEBAR_W+520, 525, 240, 36, SWP_NOZORDER);
        SetWindowPos(hBtnUploadDoc, NULL, SIDEBAR_W+520, 570, 240, 34, SWP_NOZORDER);
        ShowWindow(hDocStatusLabel, SW_SHOW);
        ShowWindow(hBtnUploadDoc, SW_SHOW);
    } else {
        ShowWindow(hBtnUploadDoc, SW_HIDE); ShowWindow(hDocStatusLabel, SW_HIDE);
    }

    populateList();
    populateVerifyList();
}

void GUI::attemptLogin(){
    char u[100],p[100];
    GetWindowText(hUserEdit,u,100); GetWindowText(hPassEdit,p,100);
    string us(u),ps(p);
    for(int i=0;i<accounts.getSize();i++){
        Account* a=*accounts.get(i);
        if(a->username==us && passwordMatches(a->password, ps)){
            if(!isHashedPassword(a->password)){ a->password=hashPassword(ps); storage->saveAccounts(accounts); }
            currentUser=a; currentState=STATE_DASHBOARD; showDashboard(); return;
        }
    }
    MessageBox(hwndMain,"Invalid username or password!","Login Failed",MB_OK|MB_ICONERROR);
}

void GUI::attemptRegister(){
    char u[100],p[100];
    GetWindowText(hRegUserEdit,u,100); GetWindowText(hRegPassEdit,p,100);
    string us(u),ps(p);
    if(isBlankText(us)||isBlankText(ps)){ MessageBox(hwndMain,"Fields cannot be empty!","Error",MB_OK|MB_ICONERROR); return; }
    if(us.size()<3){ MessageBox(hwndMain,"Username must be at least 3 characters.","Invalid Username",MB_OK|MB_ICONERROR); return; }
    if(ps.size()<4){ MessageBox(hwndMain,"Password must be at least 4 characters.","Weak Password",MB_OK|MB_ICONERROR); return; }
    for(int i=0;i<(int)us.size();i++){
        unsigned char ch=(unsigned char)us[i];
        if(!isalnum(ch) && us[i]!='_' && us[i]!='-'){
            MessageBox(hwndMain,"Username can contain only letters, numbers, underscore, and dash.","Invalid Username",MB_OK|MB_ICONERROR);
            return;
        }
    }
    if(g_pendingDocPath.empty()){ MessageBox(hwndMain,"Please attach your ID document before registering!","No Document",MB_OK|MB_ICONWARNING); return; }
    for(int i=0;i<accounts.getSize();i++){ Account* a=*accounts.get(i); if(a->username==us){ MessageBox(hwndMain,"Username taken!","Error",MB_OK|MB_ICONERROR); return; } }
    int ri=SendMessage(hRegRoleCombo,CB_GETCURSEL,0,0);
    UserRole role=(ri==1)?ROLE_SELLER:ROLE_BUYER;
    Account* newAcc=new Account(us,hashPassword(ps),role);
    newAcc->documentPath=g_pendingDocPath;
    g_pendingDocPath=""; // clear for next time
    accounts.add(newAcc);
    storage->saveAccounts(accounts);
    MessageBox(hwndMain,"Account created with your ID document!\nAdmin will verify your account before you can trade.","Success",MB_OK|MB_ICONINFORMATION);
    showLogin();
}

void GUI::logout(){ currentUser=NULL; activeSearch=""; if(hSearchEdit) SetWindowText(hSearchEdit,""); currentState=STATE_LOGIN; showLogin(); InvalidateRect(hwndMain,NULL,TRUE); }

void GUI::uploadDocument(){
    char fn[MAX_PATH]="";
    OPENFILENAME ofn={0};
    ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=hwndMain; ofn.lpstrFile=fn; ofn.nMaxFile=MAX_PATH;
    ofn.lpstrFilter="Documents\0*.pdf;*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0";
    ofn.lpstrTitle="Select Your ID Document"; ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(GetOpenFileName(&ofn)){
        if(currentState==STATE_REGISTER){
            // During registration: just store path temporarily and update label
            g_pendingDocPath=string(fn);
            string short_name=g_pendingDocPath.size()>40?"..."+g_pendingDocPath.substr(g_pendingDocPath.size()-37):g_pendingDocPath;
            SetWindowText(hDocStatusLabel,("Selected: "+short_name).c_str());
        } else if(currentUser){
            // From dashboard: update existing account and return it to pending review.
            currentUser->documentPath=string(fn);
            currentUser->status=ACC_PENDING;
            storage->saveAccounts(accounts);
            MessageBox(hwndMain,"ID Document updated. Awaiting admin approval.","Uploaded",MB_OK|MB_ICONINFORMATION);
            showDashboard();
        }
    }
}

void GUI::uploadCarFile(int id){
    char fn[MAX_PATH]="";
    OPENFILENAME ofn={0};
    ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=hwndMain; ofn.lpstrFile=fn; ofn.nMaxFile=MAX_PATH;
    ofn.lpstrFilter="Images/Docs\0*.png;*.jpg;*.jpeg;*.pdf;*.bmp\0All Files\0*.*\0";
    ofn.lpstrTitle="Select File"; ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(GetOpenFileName(&ofn)){
        string path = string(fn);
        string short_name = path.size()>20 ? "..."+path.substr(path.size()-17) : path;
        if(id==ID_BTN_CAR_IMG1){ g_pendingImg1=path; SetWindowText(hLblImg1, short_name.c_str()); }
        else if(id==ID_BTN_CAR_IMG2){ g_pendingImg2=path; SetWindowText(hLblImg2, short_name.c_str()); }
        else if(id==ID_BTN_OWNER_DOC){ g_pendingOwnerDoc=path; SetWindowText(hLblOwner, short_name.c_str()); }
        else if(id==ID_BTN_BODY_DOC){ g_pendingBodyDoc=path; SetWindowText(hLblBody, short_name.c_str()); }
    }
}

void GUI::populateVerifyList(){
    if(!hVerifyList) return;
    SendMessage(hVerifyList,LB_RESETCONTENT,0,0);
    for(int i=0;i<accounts.getSize();i++){
        Account* a=*accounts.get(i);
        if(a->role==ROLE_ADMIN) continue;
        string stat=(a->status==ACC_VERIFIED)?"[OK] ":(a->status==ACC_REJECTED)?"[X] ":"[?] ";
        string roleStr=(a->role==ROLE_SELLER)?"Seller":"Buyer";
        string hasDoc=a->documentPath.empty()?" (No doc)":" (Doc uploaded)";
        string entry=stat+a->username+" ["+roleStr+"]"+hasDoc;
        int idx=SendMessage(hVerifyList,LB_ADDSTRING,0,(LPARAM)entry.c_str());
        SendMessage(hVerifyList,LB_SETITEMDATA,idx,(LPARAM)i);
    }
}

void GUI::openSelectedDocument(){
    int sel=SendMessage(hVerifyList,LB_GETCURSEL,0,0);
    if(sel==LB_ERR){ MessageBox(hwndMain,"Select an account first.","Error",MB_OK|MB_ICONWARNING); return; }
    int accIdx=SendMessage(hVerifyList,LB_GETITEMDATA,sel,0);
    Account* a=*accounts.get(accIdx);
    if(a->documentPath.empty()){ MessageBox(hwndMain,"This user has not uploaded any document.","No Document",MB_OK|MB_ICONWARNING); return; }
    ShellExecute(NULL,"open",a->documentPath.c_str(),NULL,NULL,SW_SHOW);
}

void GUI::approveSelectedAccount(){
    int sel=SendMessage(hVerifyList,LB_GETCURSEL,0,0);
    if(sel==LB_ERR){ MessageBox(hwndMain,"Select an account to approve.","Error",MB_OK|MB_ICONWARNING); return; }
    int accIdx=SendMessage(hVerifyList,LB_GETITEMDATA,sel,0);
    Account* a=*accounts.get(accIdx);
    if(a->documentPath.empty()){
        MessageBox(hwndMain,"This account has no uploaded ID document. Reject it or ask the user to upload a document.","Missing Document",MB_OK|MB_ICONWARNING);
        return;
    }
    a->status=ACC_VERIFIED;
    storage->saveAccounts(accounts);
    populateVerifyList();
    string msg="Account '"+a->username+"' has been VERIFIED!";
    MessageBox(hwndMain,msg.c_str(),"Approved",MB_OK|MB_ICONINFORMATION);
}

void GUI::rejectSelectedAccount(){
    int sel=SendMessage(hVerifyList,LB_GETCURSEL,0,0);
    if(sel==LB_ERR){ MessageBox(hwndMain,"Select an account to reject.","Error",MB_OK|MB_ICONWARNING); return; }
    int accIdx=SendMessage(hVerifyList,LB_GETITEMDATA,sel,0);
    Account* a=*accounts.get(accIdx);
    a->status=ACC_REJECTED; a->documentPath="";
    storage->saveAccounts(accounts);
    populateVerifyList();
    string msg="Account '"+a->username+"' has been REJECTED. They must re-upload their document.";
    MessageBox(hwndMain,msg.c_str(),"Rejected",MB_OK|MB_ICONWARNING);
}

void GUI::populateList(){
    SendMessage(hwndList,LB_RESETCONTENT,0,0);
    int av=0,pend=0,sold=0,rented=0;
    string searchTerm = normalizeSearchText(activeSearch);
    for(int i=0;i<listings.getSize();i++){
        Listing* item=*listings.get(i);
        if(item->status==L_STATUS_AVAILABLE) av++;
        else if(item->status==L_STATUS_PENDING) pend++;
        else if(item->status==L_STATUS_SOLD) sold++;
        else if(item->status==L_STATUS_RENTED) rented++;
        bool show=false;
        if(currentUser->role==ROLE_ADMIN) show=true;
        else if(currentUser->role==ROLE_SELLER){ if(item->status==L_STATUS_AVAILABLE||item->ownerUsername==currentUser->username) show=true; }
        else if(currentUser->role==ROLE_BUYER){ if(item->status==L_STATUS_AVAILABLE) show=true; }

        if(show && !searchTerm.empty()){
            string searchable = item->location + " " + item->ownerUsername + " " + item->getDetails() + " " + to_string_compat(item->id);
            if(item->type==TYPE_CAR){
                Car* c=(Car*)item;
                searchable += " " + c->model + " " + c->condition;
            }
            show = normalizeSearchText(searchable).find(searchTerm) != string::npos;
        }

        if(show){
            string d=item->getDetails();
            int idx=SendMessage(hwndList,LB_ADDSTRING,0,(LPARAM)d.c_str());
            SendMessage(hwndList,LB_SETITEMDATA,idx,(LPARAM)item->id);
        }
    }
    string rep="Available: "+to_string_compat(av)+"\r\n";
    rep+="Pending:   "+to_string_compat(pend)+"\r\n";
    rep+="Sold:      "+to_string_compat(sold)+"\r\n";
    rep+="Rented:    "+to_string_compat(rented)+"\r\n";
    rep+="Orders:    "+to_string_compat(orderQueue.getSize())+"\r\n";
    rep+="Commission: ETB "+to_string_compat((int)totalCommission);
    SetWindowText(hReportBox,rep.c_str());
}

void GUI::applySearch(){
    char q[256];
    GetWindowText(hSearchEdit,q,256);
    activeSearch=string(q);
    if(isBlankText(activeSearch)) activeSearch="";
    populateList();
}

void GUI::clearSearch(){
    activeSearch="";
    SetWindowText(hSearchEdit,"");
    populateList();
}

bool GUI::ensureCurrentUserCanTrade(const char* action){
    if(!currentUser) return false;
    if(currentUser->role==ROLE_ADMIN || currentUser->status==ACC_VERIFIED) return true;

    string status = (currentUser->status==ACC_REJECTED) ? "rejected" : "pending";
    string msg = string("Your account is ") + status + ". Admin verification is required before you can " + string(action) + ".";
    if(currentUser->status==ACC_REJECTED) msg += "\nUpload a new ID document from the dashboard and wait for admin approval.";
    MessageBox(hwndMain,msg.c_str(),"Verification Required",MB_OK|MB_ICONWARNING);
    return false;
}

int GUI::generateNextListingId(){
    int maxId=0;
    for(int i=0;i<listings.getSize();i++){
        Listing* item=*listings.get(i);
        if(item && item->id>maxId) maxId=item->id;
    }
    return maxId+1;
}

void GUI::addListing(){
    if(!ensureCurrentUserCanTrade("add listings")) return;
    char pr[256],lo[256],p1[256],p2[256];
    GetWindowText(hPrice,pr,256); GetWindowText(hLocation,lo,256);
    GetWindowText(hParam1,p1,256); GetWindowText(hParam2,p2,256);
    int type=SendMessage(hTypeCombo,CB_GETCURSEL,0,0);
    if(isBlankText(string(pr))||isBlankText(string(lo))){ MessageBox(hwndMain,"Price and Location required!","Error",MB_OK|MB_ICONERROR); return; }
    double priceValue=0;
    if(!parsePositiveDouble(pr, priceValue)){ MessageBox(hwndMain,"Price must be a valid number greater than zero.","Invalid Price",MB_OK|MB_ICONERROR); return; }
    
    int nid=generateNextListingId();
    if(type==0){
        int bedrooms=0;
        double sizeValue=0;
        if(!parsePositiveInt(p1, bedrooms) || !parsePositiveDouble(p2, sizeValue)){ MessageBox(hwndMain,"House bedrooms and size must be valid numbers greater than zero.","Invalid House Details",MB_OK|MB_ICONERROR); return; }
        House* h=new House(); h->id=nid; h->price=priceValue; h->location=lo; h->bedrooms=bedrooms; h->size=sizeValue; h->status=L_STATUS_PENDING; h->ownerUsername=currentUser->username;
        listings.add(h); bst.insert(h); hashTable.insert(h->id,h);
    }
    else{
        if(isBlankText(string(p1)) || isBlankText(string(p2))){
            MessageBox(hwndMain,"Car model and condition are required.","Invalid Car Details",MB_OK|MB_ICONERROR);
            return;
        }
        if(g_pendingImg1=="" || g_pendingImg2==""){
            MessageBox(hwndMain,"Sellers must upload at least 2 images for car listings!","Missing Images",MB_OK|MB_ICONWARNING);
            return;
        }
        if(g_pendingOwnerDoc=="" || g_pendingBodyDoc==""){
            MessageBox(hwndMain,"Sellers must upload ownership and body documents!","Missing Documents",MB_OK|MB_ICONWARNING);
            return;
        }
        Car* c=new Car(); c->id=nid; c->price=priceValue; c->location=lo; c->model=p1; c->condition=p2;
        c->imgPath1 = g_pendingImg1; c->imgPath2 = g_pendingImg2;
        c->ownershipDocPath = g_pendingOwnerDoc; c->bodyDocPath = g_pendingBodyDoc;
        c->status=L_STATUS_PENDING; c->ownerUsername=currentUser->username;
        listings.add(c); bst.insert(c); hashTable.insert(c->id,c);
    }
    storage->saveListings(listings); populateList();
    
    // Reset car fields
    g_pendingImg1 = g_pendingImg2 = g_pendingOwnerDoc = g_pendingBodyDoc = "";
    SetWindowText(hLblImg1, "Car Image 1"); SetWindowText(hLblImg2, "Car Image 2");
    SetWindowText(hLblOwner, "Ownership Doc"); SetWindowText(hLblBody, "Body Doc");

    MessageBox(hwndMain,"Listing added! Pending admin approval.","Success",MB_OK|MB_ICONINFORMATION);
    SetWindowText(hPrice,""); SetWindowText(hLocation,""); SetWindowText(hParam1,""); SetWindowText(hParam2,"");
    showDashboard();
}

void GUI::buyListing(){
    if(!ensureCurrentUserCanTrade("place orders")) return;
    int sel=SendMessage(hwndList,LB_GETCURSEL,0,0);
    if(sel==LB_ERR){ MessageBox(hwndMain,"Select an item first.","Error",MB_OK|MB_ICONWARNING); return; }
    int lid=SendMessage(hwndList,LB_GETITEMDATA,sel,0);
    Listing* item=(Listing*)hashTable.search(lid);
    if(!item||item->status!=L_STATUS_AVAILABLE){ MessageBox(hwndMain,"Item not available.","Error",MB_OK|MB_ICONERROR); return; }

    bool alreadyQueued=false;
    Queue<Inquiry*> tempQ;
    while(!orderQueue.isEmpty()){
        Inquiry* existing=orderQueue.dequeue();
        if(existing && existing->listingId==item->id) alreadyQueued=true;
        tempQ.enqueue(existing);
    }
    while(!tempQ.isEmpty()) orderQueue.enqueue(tempQ.dequeue());
    if(alreadyQueued){ MessageBox(hwndMain,"This listing already has a pending order. Please wait for admin processing.","Order Already Pending",MB_OK|MB_ICONINFORMATION); return; }

    orderQueue.enqueue(new Inquiry(item->id,currentUser->username));
    storage->saveOrders(orderQueue); populateList();
    MessageBox(hwndMain,"Order placed! Admin will process it.","Success",MB_OK|MB_ICONINFORMATION);
}

void GUI::approveListing(){
    int sel=SendMessage(hwndList,LB_GETCURSEL,0,0);
    if(sel==LB_ERR){ MessageBox(hwndMain,"Select a pending item.","Error",MB_OK|MB_ICONWARNING); return; }
    int lid=SendMessage(hwndList,LB_GETITEMDATA,sel,0);
    Listing* item=(Listing*)hashTable.search(lid);
    if(!item||item->status!=L_STATUS_PENDING){ MessageBox(hwndMain,"Item is not pending.","Error",MB_OK|MB_ICONERROR); return; }
    Account* ownerAcc=NULL;
    for(int i=0;i<accounts.getSize();i++){
        Account* a=*accounts.get(i);
        if(a && a->username==item->ownerUsername){ ownerAcc=a; break; }
    }
    if(!ownerAcc && !item->ownerUsername.empty()){
        MessageBox(hwndMain,"The listing owner account could not be found. Restore or recreate the seller account before approval.","Owner Missing",MB_OK|MB_ICONWARNING);
        return;
    }
    if(ownerAcc && ownerAcc->role!=ROLE_ADMIN && ownerAcc->status!=ACC_VERIFIED){
        MessageBox(hwndMain,"The listing owner is not verified. Verify the seller account before approving this listing.","Owner Not Verified",MB_OK|MB_ICONWARNING);
        return;
    }
    if(item->type==TYPE_CAR){
        Car* c=(Car*)item;
        if(c->imgPath1.empty() || c->imgPath2.empty() || c->ownershipDocPath.empty() || c->bodyDocPath.empty()){
            MessageBox(hwndMain,"Car listings must have two images, ownership document, and body document before approval.","Missing Car Documents",MB_OK|MB_ICONWARNING);
            return;
        }
    }
    item->status=L_STATUS_AVAILABLE; storage->saveListings(listings); populateList();
    MessageBox(hwndMain,"Listing approved and visible to buyers!","Success",MB_OK|MB_ICONINFORMATION);
}

void GUI::processOrder(){
    if(orderQueue.isEmpty()){ MessageBox(hwndMain,"No pending orders.","Info",MB_OK|MB_ICONINFORMATION); return; }
    Inquiry* inq=orderQueue.peek();
    Listing* item=(Listing*)hashTable.search(inq->listingId);
    if(!item){ delete orderQueue.dequeue(); storage->saveOrders(orderQueue); populateList(); return; }
    if(item->status!=L_STATUS_AVAILABLE){
        delete orderQueue.dequeue();
        storage->saveOrders(orderQueue);
        populateList();
        MessageBox(hwndMain,"This queued order was skipped because the listing is no longer available.","Order Skipped",MB_OK|MB_ICONINFORMATION);
        return;
    }
    double commission=item->price*COMMISSION_RATE;
    double sellerAmount=item->price-commission;
    bool isPaid = inq->message.find("PAID:") == 0;
    string msg="Buyer '"+inq->clientName+"' wants Listing ID "+to_string_compat(item->id)+
        "\nLocation: "+item->location+
        "\nPrice paid: ETB "+to_string_compat((int)item->price)+
        "\nBroker/System commission (5%): ETB "+to_string_compat((int)commission)+
        "\nSeller receives: ETB "+to_string_compat((int)sellerAmount);
    if(!isPaid) msg += "\n\nNote: this order was requested but not marked as paid.";
    else msg += "\n\nPayment is recorded. Approve transfer to seller?";
    if(MessageBox(hwndMain,msg.c_str(),"Process Order",MB_YESNO|MB_ICONQUESTION)==IDYES){
        totalCommission+=commission;
        item->status=(item->type==TYPE_HOUSE)?L_STATUS_SOLD:L_STATUS_RENTED;
        storage->saveListings(listings);
        delete orderQueue.dequeue(); storage->saveOrders(orderQueue);
        populateList();
        string result="Sale Approved!\n\nSale Price:   ETB "+to_string_compat((int)item->price)+
                      "\nCommission (5%): ETB "+to_string_compat((int)commission)+
                      "\nSeller Receives: ETB "+to_string_compat((int)sellerAmount)+
                      "\n\nTotal Commission Earned: ETB "+to_string_compat((int)totalCommission);
        MessageBox(hwndMain,result.c_str(),"Commission Calculated",MB_OK|MB_ICONINFORMATION);
    } else { delete orderQueue.dequeue(); storage->saveOrders(orderQueue); populateList(); }
}

void GUI::viewListingDetails(){
    if(!selectedListing) return;
    string typeStr = (selectedListing->type==TYPE_HOUSE) ? "House" : "Car";
    string statusStr = (selectedListing->status==L_STATUS_PENDING) ? "Pending" :
        (selectedListing->status==L_STATUS_AVAILABLE) ? "Available" :
        (selectedListing->status==L_STATUS_SOLD) ? "Sold" : "Rented";

    string msg = "Listing ID: " + to_string_compat(selectedListing->id) +
                 "\nType: " + typeStr +
                 "\nStatus: " + statusStr +
                 "\nPrice: ETB " + to_string_compat((int)selectedListing->price) +
                 "\nLocation: " + selectedListing->location +
                 "\nSeller: " + selectedListing->ownerUsername;

    if(selectedListing->type==TYPE_HOUSE){
        House* h=(House*)selectedListing;
        msg += "\nBedrooms: " + to_string_compat(h->bedrooms) +
               "\nSize: " + to_string_compat((int)h->size) + " sqm";
    } else {
        Car* c=(Car*)selectedListing;
        msg += "\nModel: " + c->model +
               "\nCondition: " + c->condition;
        if(currentUser->role==ROLE_ADMIN || currentUser->username==selectedListing->ownerUsername){
            msg += "\nImage 1: " + c->imgPath1 +
                   "\nImage 2: " + c->imgPath2 +
                   "\nOwnership Doc: " + c->ownershipDocPath +
                   "\nBody Doc: " + c->bodyDocPath;
        }
    }

    MessageBox(hwndMain, msg.c_str(), "Listing Details", MB_OK|MB_ICONINFORMATION);
}

void GUI::updateDetailPanel(Listing* item){
    selectedListing = item;
    if(!item) return;

    if(currentUser->role != ROLE_BUYER){
        return;
    }

    string title = (item->type == TYPE_HOUSE) ? "Modern House" : ((Car*)item)->model;
    string price = "ETB " + to_string_compat((int)item->price);
    
    SetWindowText(hTitleLbl, title.c_str());
    SetWindowText(hPriceLbl, price.c_str());
    
    ShowWindow(hDetailPanel, SW_SHOW);
    ShowWindow(hMainImgCtrl, SW_SHOW);
    ShowWindow(hTitleLbl, SW_SHOW);
    ShowWindow(hPriceLbl, SW_SHOW);
    ShowWindow(hContactBtn, SW_SHOW);
    
    // Only buyers can record payment for an available listing.
    if(currentUser->role == ROLE_BUYER){
        ShowWindow(hBtnPay, SW_SHOW);
    } else {
        ShowWindow(hBtnPay, SW_HIDE);
    }

    // Load and display image
    string imgPath = "";
    if(item->type == TYPE_CAR){ imgPath = ((Car*)item)->imgPath1; }
    
    if(!imgPath.empty()){
        // Convert string to wstring for GDI+
        int len = MultiByteToWideChar(CP_UTF8, 0, imgPath.c_str(), -1, NULL, 0);
        wchar_t* wpath = new wchar_t[len];
        MultiByteToWideChar(CP_UTF8, 0, imgPath.c_str(), -1, wpath, len);
        
        Bitmap bmp(wpath);
        delete[] wpath;
        
        HBITMAP hBmp = NULL;
        bmp.GetHBITMAP(Color(255,255,255), &hBmp);
        HBITMAP oldBmp = (HBITMAP)SendMessage(hMainImgCtrl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
        if(oldBmp) DeleteObject(oldBmp);
    } else {
        HBITMAP oldBmp = (HBITMAP)SendMessage(hMainImgCtrl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
        if(oldBmp) DeleteObject(oldBmp);
    }
}

void GUI::pay(){
    if(!ensureCurrentUserCanTrade("make payments")) return;
    int sel=SendMessage(hwndList,LB_GETCURSEL,0,0);
    if(sel==LB_ERR){ MessageBox(hwndMain,"Select an item to pay for.","Error",MB_OK|MB_ICONWARNING); return; }
    int lid=SendMessage(hwndList,LB_GETITEMDATA,sel,0);
    Listing* item=(Listing*)hashTable.search(lid);
    if(!item){ return; }
    if(item->status!=L_STATUS_AVAILABLE){ MessageBox(hwndMain,"Only available listings can be paid for.","Not Available",MB_OK|MB_ICONWARNING); return; }

    double commission=item->price*COMMISSION_RATE;
    double sellerAmount=item->price-commission;
    string confirm = "Payment Summary\n\nListing ID: " + to_string_compat(item->id) +
                     "\nBuyer pays: ETB " + to_string_compat((int)item->price) +
                     "\nBroker/System commission (5%): ETB " + to_string_compat((int)commission) +
                     "\nSeller receives after admin approval: ETB " + to_string_compat((int)sellerAmount) +
                     "\n\nConfirm payment?";
    if(MessageBox(hwndMain, confirm.c_str(), "Confirm Payment", MB_YESNO|MB_ICONQUESTION) != IDYES) return;

    bool alreadyQueued=false;
    Queue<Inquiry*> tempQ;
    while(!orderQueue.isEmpty()){
        Inquiry* existing=orderQueue.dequeue();
        if(existing && existing->listingId==item->id) alreadyQueued=true;
        tempQ.enqueue(existing);
    }
    while(!tempQ.isEmpty()) orderQueue.enqueue(tempQ.dequeue());

    if(!alreadyQueued){
        Inquiry* inq = new Inquiry(item->id, currentUser->username);
        inq->message = "PAID: Buyer paid ETB " + to_string_compat((int)item->price) +
                       ". Broker commission ETB " + to_string_compat((int)commission) +
                       ". Seller payout ETB " + to_string_compat((int)sellerAmount) +
                       ". Waiting for admin approval.";
        orderQueue.enqueue(inq);
        storage->saveOrders(orderQueue);
        populateList();
    }

    string done = "Payment recorded successfully.\n\n";
    done += "Buyer: " + currentUser->username +
            "\nSeller: " + item->ownerUsername +
            "\nAmount paid: ETB " + to_string_compat((int)item->price) +
            "\nBroker/System commission: ETB " + to_string_compat((int)commission) +
            "\nSeller payout after admin approval: ETB " + to_string_compat((int)sellerAmount);
    if(alreadyQueued) done += "\n\nThis listing already had a pending order, so no duplicate order was created.";
    else done += "\n\nAdmin must approve the transaction before the seller receives the money.";
    MessageBox(hwndMain, done.c_str(), "Payment Complete", MB_OK|MB_ICONINFORMATION);
}

void GUI::deleteSelectedListing(){
    int sel=SendMessage(hwndList,LB_GETCURSEL,0,0);
    if(sel==LB_ERR){ MessageBox(hwndMain,"Select an item to delete.","Error",MB_OK|MB_ICONWARNING); return; }
    int lid=SendMessage(hwndList,LB_GETITEMDATA,sel,0);
    Listing* removed=NULL;
    for(int i=0;i<listings.getSize();i++){
        Listing* it=*listings.get(i);
        if(it->id==lid){ removed=it; hashTable.remove(lid); listings.remove(i); break; }
    }
    if(!removed){ MessageBox(hwndMain,"Listing was not found.","Delete Failed",MB_OK|MB_ICONERROR); populateList(); return; }
    bst.clear(); for(int i=0;i<listings.getSize();i++) bst.insert(*listings.get(i));
    if(selectedListing==removed){ selectedListing=NULL; ShowWindow(hDetailPanel,SW_HIDE); ShowWindow(hMainImgCtrl,SW_HIDE); ShowWindow(hTitleLbl,SW_HIDE); ShowWindow(hPriceLbl,SW_HIDE); ShowWindow(hContactBtn,SW_HIDE); ShowWindow(hBtnPay,SW_HIDE); }
    Queue<Inquiry*> keptOrders;
    while(!orderQueue.isEmpty()){
        Inquiry* inq=orderQueue.dequeue();
        if(inq && inq->listingId==lid) delete inq;
        else keptOrders.enqueue(inq);
    }
    while(!keptOrders.isEmpty()) orderQueue.enqueue(keptOrders.dequeue());
    delete removed;
    storage->saveListings(listings); storage->saveOrders(orderQueue); populateList();
    MessageBox(hwndMain,"Listing deleted!","Success",MB_OK|MB_ICONINFORMATION);
}

LRESULT CALLBACK GUI::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){
    case WM_CREATE: {
        int SB=SIDEBAR_W;
        int LX=LOGIN_HALF_W+80; // Login X
        // Logo panel
        g_GUI->hLogoControl=CreateWindow("STATIC","",WS_CHILD|SS_BITMAP|SS_REALSIZEIMAGE,0,0,LOGIN_HALF_W,680,hwnd,NULL,g_GUI->hInstance,NULL);
        if(g_GUI->hLogoImage) SendMessage(g_GUI->hLogoControl,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)g_GUI->hLogoImage);

        // LOGIN (Half-Half)
        g_GUI->hLoginTitle=CreateWindow("STATIC","Welcome Back",WS_CHILD|SS_CENTER,LX,140,360,40,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hLoginSubtitle=CreateWindow("STATIC","Manage your listings with ease",WS_CHILD|SS_CENTER,LX,185,360,22,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hUserLabel=CreateWindow("STATIC","Username",WS_CHILD,LX+30,225,300,20,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hUserEdit=CreateWindow("EDIT","",WS_CHILD|WS_BORDER,LX+30,248,300,32,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hPassLabel=CreateWindow("STATIC","Password",WS_CHILD,LX+30,295,300,20,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hPassEdit=CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,LX+30,318,300,32,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hBtnLogin=CreateWindow("BUTTON","Sign In",WS_CHILD|BS_OWNERDRAW,LX+30,370,300,42,hwnd,(HMENU)ID_BTN_LOGIN,g_GUI->hInstance,NULL);
        g_GUI->hBtnGoToRegister=CreateWindow("BUTTON","Create an Account",WS_CHILD|BS_OWNERDRAW,LX+30,425,300,34,hwnd,(HMENU)ID_BTN_GOTO_REG,g_GUI->hInstance,NULL);

        // REGISTER (Half-Half)
        g_GUI->hRegTitle=CreateWindow("STATIC","Join Now",WS_CHILD|SS_CENTER,LX,80,360,40,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hRegSubtitle=CreateWindow("STATIC","Start your brokerage journey",WS_CHILD|SS_CENTER,LX,122,360,20,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hRegUserLabel=CreateWindow("STATIC","Username",WS_CHILD,LX+30,152,300,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hRegUserEdit=CreateWindow("EDIT","",WS_CHILD|WS_BORDER,LX+30,172,300,30,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hRegPassLabel=CreateWindow("STATIC","Password",WS_CHILD,LX+30,210,300,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hRegPassEdit=CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_PASSWORD,LX+30,230,300,30,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hRegRoleLabel=CreateWindow("STATIC","Your Role",WS_CHILD,LX+30,268,300,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hRegRoleCombo=CreateWindow("COMBOBOX",NULL,WS_CHILD|CBS_DROPDOWNLIST,LX+30,288,300,100,hwnd,(HMENU)ID_COMBO_ROLE,g_GUI->hInstance,NULL);
        SendMessage(g_GUI->hRegRoleCombo,CB_ADDSTRING,0,(LPARAM)"  Buyer"); SendMessage(g_GUI->hRegRoleCombo,CB_ADDSTRING,0,(LPARAM)"  Seller");
        g_GUI->hDocStatusLabel=CreateWindow("STATIC","No file attached",WS_CHILD,LX+30,335,300,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hBtnUploadDoc=CreateWindow("BUTTON","Attach ID Document",WS_CHILD|BS_OWNERDRAW,LX+30,355,300,32,hwnd,(HMENU)ID_BTN_UPLOAD,g_GUI->hInstance,NULL);
        g_GUI->hBtnRegister=CreateWindow("BUTTON","Register Account",WS_CHILD|BS_OWNERDRAW,LX+30,405,300,42,hwnd,(HMENU)ID_BTN_REGISTER,g_GUI->hInstance,NULL);
        g_GUI->hBtnBackToLogin=CreateWindow("BUTTON","Back to Login",WS_CHILD|BS_OWNERDRAW,LX+30,458,300,30,hwnd,(HMENU)ID_BTN_BACK_LOGIN,g_GUI->hInstance,NULL);

        // DASHBOARD
        g_GUI->hDashTitle=CreateWindow("STATIC","  Broker Portal",WS_CHILD,SB+15,12,400,35,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hUserWelcome=CreateWindow("STATIC","",WS_CHILD,SB+450,18,350,25,hwnd,NULL,g_GUI->hInstance,NULL);

        // SIDEBAR
        g_GUI->hBtnBuy    =CreateWindow("BUTTON","Marketplace",       WS_CHILD|BS_OWNERDRAW,0,80 ,SB,48,hwnd,(HMENU)ID_BTN_BUY    ,g_GUI->hInstance,NULL);
        g_GUI->hBtnSort   =CreateWindow("BUTTON","Sort Assets",       WS_CHILD|BS_OWNERDRAW,0,136,SB,48,hwnd,(HMENU)ID_BTN_SORT   ,g_GUI->hInstance,NULL);
        g_GUI->hBtnApprove=CreateWindow("BUTTON","Approve Listing",   WS_CHILD|BS_OWNERDRAW,0,192,SB,48,hwnd,(HMENU)ID_BTN_APPROVE ,g_GUI->hInstance,NULL);
        g_GUI->hBtnDelete =CreateWindow("BUTTON","Delete Entry",      WS_CHILD|BS_OWNERDRAW,0,248,SB,48,hwnd,(HMENU)ID_BTN_DELETE ,g_GUI->hInstance,NULL);
        g_GUI->hBtnProcess=CreateWindow("BUTTON","Orders",            WS_CHILD|BS_OWNERDRAW,0,304,SB,48,hwnd,(HMENU)ID_BTN_PROCESS,g_GUI->hInstance,NULL);
        g_GUI->hBtnLogout =CreateWindow("BUTTON","Sign Out",          WS_CHILD|BS_OWNERDRAW,0,590,SB,48,hwnd,(HMENU)ID_BTN_LOGOUT ,g_GUI->hInstance,NULL);


        // CENTER search and listing area
        g_GUI->hSearchEdit=CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_AUTOHSCROLL,SB+20,60,280,28,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hBtnSearch=CreateWindow("BUTTON","Search",WS_CHILD|BS_OWNERDRAW,SB+315,58,85,32,hwnd,(HMENU)ID_BTN_SEARCH,g_GUI->hInstance,NULL);
        g_GUI->hBtnResetSearch=CreateWindow("BUTTON","Reset",WS_CHILD|BS_OWNERDRAW,SB+410,58,90,32,hwnd,(HMENU)ID_BTN_RESET_SEARCH,g_GUI->hInstance,NULL);
        g_GUI->hwndList=CreateWindow("LISTBOX",NULL,
            WS_CHILD|WS_VSCROLL|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT,
            SB+20,100,480,340,hwnd,(HMENU)ID_LISTBOX,g_GUI->hInstance,NULL);
        SendMessage(g_GUI->hwndList,LB_SETITEMHEIGHT,0,(LPARAM)CARD_H);

        // RIGHT Panel
        int RX=SB+520;
        g_GUI->hLblType=CreateWindow("STATIC","Category",WS_CHILD,RX,80,240,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hTypeCombo=CreateWindow("COMBOBOX",NULL,WS_CHILD|CBS_DROPDOWNLIST,RX,100,240,100,hwnd,(HMENU)ID_COMBO_TYPE,g_GUI->hInstance,NULL);
        SendMessage(g_GUI->hTypeCombo,CB_ADDSTRING,0,(LPARAM)"House"); SendMessage(g_GUI->hTypeCombo,CB_ADDSTRING,0,(LPARAM)"Car"); SendMessage(g_GUI->hTypeCombo,CB_SETCURSEL,0,0);
        g_GUI->hLblPrice=CreateWindow("STATIC","Price",WS_CHILD,RX,140,240,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hPrice=CreateWindow("EDIT","",WS_CHILD|WS_BORDER,RX,160,240,30,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hLblLoc=CreateWindow("STATIC","Location",WS_CHILD,RX,200,240,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hLocation=CreateWindow("EDIT","",WS_CHILD|WS_BORDER,RX,220,240,30,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hLblP1=CreateWindow("STATIC","Details 1",WS_CHILD,RX,260,240,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hParam1=CreateWindow("EDIT","",WS_CHILD|WS_BORDER,RX,280,240,30,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hLblP2=CreateWindow("STATIC","Details 2",WS_CHILD,RX,320,240,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hParam2=CreateWindow("EDIT","",WS_CHILD|WS_BORDER,RX,340,240,30,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hBtnAdd=CreateWindow("BUTTON","Add to Market",WS_CHILD|BS_OWNERDRAW,RX,385,240,38,hwnd,(HMENU)ID_BTN_ADD,g_GUI->hInstance,NULL);

        // Reports
        g_GUI->hReportTitle=CreateWindow("STATIC","Statistics",WS_CHILD,RX,440,240,22,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hReportBox=CreateWindow("EDIT","",WS_CHILD|WS_BORDER|ES_MULTILINE|ES_READONLY,RX,465,240,130,hwnd,NULL,g_GUI->hInstance,NULL);

        // Admin verification
        g_GUI->hVerifyTitle=CreateWindow("STATIC","User Verifications",WS_CHILD,SB+20,460,300,22,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hVerifyList=CreateWindow("LISTBOX",NULL,WS_CHILD|WS_BORDER|WS_VSCROLL|LBS_NOTIFY,SB+20,485,350,140,hwnd,(HMENU)ID_VERIFY_LIST,g_GUI->hInstance,NULL);
        g_GUI->hBtnOpenDoc  =CreateWindow("BUTTON","View ID",    WS_CHILD|BS_OWNERDRAW,SB+385,485,110,34,hwnd,(HMENU)ID_BTN_OPEN_DOC  ,g_GUI->hInstance,NULL);
        g_GUI->hBtnApproveAcc=CreateWindow("BUTTON","Approve",    WS_CHILD|BS_OWNERDRAW,SB+385,530,110,34,hwnd,(HMENU)ID_BTN_APPROVE_ACC,g_GUI->hInstance,NULL);
        g_GUI->hBtnRejectAcc =CreateWindow("BUTTON","Reject",     WS_CHILD|BS_OWNERDRAW,SB+385,575,110,34,hwnd,(HMENU)ID_BTN_REJECT_ACC ,g_GUI->hInstance,NULL);

        // Car Specific Form Controls (Stacked under Details 2)
        int CX = RX;
        g_GUI->hLblImg1 = CreateWindow("STATIC","Car Image 1",WS_CHILD,CX,380,115,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hBtnCarImg1 = CreateWindow("BUTTON","Upload",WS_CHILD|BS_OWNERDRAW,CX,400,115,28,hwnd,(HMENU)ID_BTN_CAR_IMG1,g_GUI->hInstance,NULL);
        g_GUI->hLblImg2 = CreateWindow("STATIC","Car Image 2",WS_CHILD,CX+125,380,115,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hBtnCarImg2 = CreateWindow("BUTTON","Upload",WS_CHILD|BS_OWNERDRAW,CX+125,400,115,28,hwnd,(HMENU)ID_BTN_CAR_IMG2,g_GUI->hInstance,NULL);
        
        g_GUI->hLblOwner = CreateWindow("STATIC","Ownership Doc",WS_CHILD,CX,435,115,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hBtnOwnerDoc = CreateWindow("BUTTON","Upload",WS_CHILD|BS_OWNERDRAW,CX,455,115,28,hwnd,(HMENU)ID_BTN_OWNER_DOC,g_GUI->hInstance,NULL);
        g_GUI->hLblBody = CreateWindow("STATIC","Body Doc",WS_CHILD,CX+125,435,115,18,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hBtnBodyDoc = CreateWindow("BUTTON","Upload",WS_CHILD|BS_OWNERDRAW,CX+125,455,115,28,hwnd,(HMENU)ID_BTN_BODY_DOC,g_GUI->hInstance,NULL);

        // Reposition Add button lower if car
        SetWindowPos(g_GUI->hBtnAdd, NULL, RX, 495, 240, 38, SWP_NOZORDER);

        // Buyer Jiji-style Detail Panel (replaces right panel for buyers)
        RX = SB + 520;
        g_GUI->hDetailPanel = CreateWindow("STATIC","",WS_CHILD,RX,80,240,550,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hMainImgCtrl = CreateWindow("STATIC","",WS_CHILD|WS_BORDER|SS_BITMAP,RX,100,240,160,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hTitleLbl    = CreateWindow("STATIC","Select an item",WS_CHILD,RX,270,240,40,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hPriceLbl    = CreateWindow("STATIC","",WS_CHILD,RX,315,240,25,hwnd,NULL,g_GUI->hInstance,NULL);
        g_GUI->hBtnDetails  = CreateWindow("BUTTON","Show Contact",WS_CHILD|BS_OWNERDRAW,RX,350,240,40,hwnd,(HMENU)ID_BTN_DETAILS,g_GUI->hInstance,NULL);
        g_GUI->hContactBtn = g_GUI->hBtnDetails;
        g_GUI->hBtnPay = CreateWindow("BUTTON","Make Payment",WS_CHILD|BS_OWNERDRAW,RX,400,240,40,hwnd,(HMENU)ID_BTN_PAY,g_GUI->hInstance,NULL);

        // Styling
        HWND fn[]={g_GUI->hUserLabel,g_GUI->hUserEdit,g_GUI->hPassLabel,g_GUI->hPassEdit,g_GUI->hBtnLogin,g_GUI->hBtnGoToRegister,
            g_GUI->hRegUserLabel,g_GUI->hRegUserEdit,g_GUI->hRegPassLabel,g_GUI->hRegPassEdit,g_GUI->hRegRoleLabel,g_GUI->hRegRoleCombo,
            g_GUI->hDocStatusLabel,g_GUI->hBtnUploadDoc,g_GUI->hBtnRegister,g_GUI->hBtnBackToLogin,
            g_GUI->hUserWelcome,g_GUI->hSearchEdit,g_GUI->hBtnSearch,g_GUI->hBtnResetSearch,g_GUI->hwndList,g_GUI->hTypeCombo,g_GUI->hPrice,g_GUI->hLocation,
            g_GUI->hParam1,g_GUI->hParam2,g_GUI->hLblType,g_GUI->hLblPrice,g_GUI->hLblLoc,g_GUI->hLblP1,g_GUI->hLblP2,
            g_GUI->hBtnAdd,g_GUI->hReportTitle,g_GUI->hReportBox,g_GUI->hLoginSubtitle,g_GUI->hRegSubtitle,
            g_GUI->hVerifyTitle,g_GUI->hVerifyList,g_GUI->hBtnOpenDoc,g_GUI->hBtnApproveAcc,g_GUI->hBtnRejectAcc,
            g_GUI->hLblImg1,g_GUI->hBtnCarImg1,g_GUI->hLblImg2,g_GUI->hBtnCarImg2,g_GUI->hLblOwner,g_GUI->hBtnOwnerDoc,g_GUI->hLblBody,g_GUI->hBtnBodyDoc,
            g_GUI->hBtnDetails,g_GUI->hBtnPay, g_GUI->hTitleLbl, g_GUI->hPriceLbl, g_GUI->hContactBtn};
        for(int i=0;i<(int)(sizeof(fn)/sizeof(HWND));i++) SendMessage(fn[i],WM_SETFONT,(WPARAM)g_GUI->hFontNormal,TRUE);
        SendMessage(g_GUI->hLoginTitle,WM_SETFONT,(WPARAM)g_GUI->hFontTitle,TRUE);
        SendMessage(g_GUI->hRegTitle,WM_SETFONT,(WPARAM)g_GUI->hFontTitle,TRUE);
        SendMessage(g_GUI->hDashTitle,WM_SETFONT,(WPARAM)g_GUI->hFontBold,TRUE);
        g_GUI->showLogin();
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc=BeginPaint(hwnd,&ps);
        RECT rc; GetClientRect(hwnd,&rc);
        if(g_GUI->currentState==STATE_DASHBOARD) g_GUI->paintBackground(hwnd,hdc);
        else {
            HBRUSH leftBrush=CreateSolidBrush(RGB(14, 116, 86));
            HBRUSH rightBrush=CreateSolidBrush(RGB(248, 250, 252));
            RECT left={0,0,LOGIN_HALF_W,rc.bottom}; FillRect(hdc,&left,leftBrush);
            RECT right={LOGIN_HALF_W,0,rc.right,rc.bottom}; FillRect(hdc,&right,rightBrush);
            DeleteObject(leftBrush); DeleteObject(rightBrush);

            RECT card={LOGIN_HALF_W+54,72,rc.right-54,rc.bottom-72};
            drawPanel(hdc,card);
            SetBkMode(hdc,TRANSPARENT);
            SelectObject(hdc,g_GUI->hFontTitle);
            SetTextColor(hdc,RGB(255,255,255));
            RECT brand={48,70,LOGIN_HALF_W-48,118};
            DrawText(hdc,"Broker Portal",-1,&brand,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
            SelectObject(hdc,g_GUI->hFontNormal);
            SetTextColor(hdc,RGB(211, 247, 229));
            RECT tagline={50,118,LOGIN_HALF_W-52,170};
            DrawText(hdc,"A cleaner way to manage listings, verifications, orders, and sales.",-1,&tagline,DT_LEFT|DT_WORDBREAK);
        }
        EndPaint(hwnd,&ps); return 0;
    }
    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis=(LPDRAWITEMSTRUCT)lParam;
        int id=dis->CtlID;
        // Sidebar buttons
        if(id==ID_BTN_BUY)     g_GUI->drawSidebarButton(dis, g_GUI->currentUser && g_GUI->currentUser->role==ROLE_BUYER ? "  Place Order" : "  Marketplace");
        else if(id==ID_BTN_SORT)     g_GUI->drawSidebarButton(dis,"  Sort Assets");
        else if(id==ID_BTN_APPROVE)  g_GUI->drawSidebarButton(dis,"  Verify Listing");
        else if(id==ID_BTN_DELETE)   g_GUI->drawSidebarButton(dis,"  Remove Entry");
        else if(id==ID_BTN_PROCESS)  g_GUI->drawSidebarButton(dis,"  Sales Queue");
        else if(id==ID_BTN_LOGOUT)   g_GUI->drawSidebarButton(dis,"  Sign Out");
        // Modernized Main Buttons
        else if(id==ID_BTN_LOGIN)    g_GUI->drawCustomButton(dis, "Sign In", ACCENT_CLR);
        else if(id==ID_BTN_GOTO_REG) g_GUI->drawCustomButton(dis, "Create Account", RGB(100, 116, 139));
        else if(id==ID_BTN_REGISTER) g_GUI->drawCustomButton(dis, "Sign Up", ACCENT_CLR);
        else if(id==ID_BTN_BACK_LOGIN) g_GUI->drawCustomButton(dis, "Back to Login", RGB(100, 116, 139));
        else if(id==ID_BTN_UPLOAD)   g_GUI->drawCustomButton(dis, "Attach Document", RGB(71, 85, 105));
        else if(id==ID_BTN_ADD)      g_GUI->drawCustomButton(dis, "Add to Marketplace", ACCENT_CLR);
        else if(id==ID_BTN_OPEN_DOC) g_GUI->drawCustomButton(dis, "View ID", RGB(71, 85, 105));
        else if(id==ID_BTN_APPROVE_ACC) g_GUI->drawCustomButton(dis, "Approve", GREEN_CLR);
        else if(id==ID_BTN_REJECT_ACC) g_GUI->drawCustomButton(dis, "Reject", RED_CLR);
        else if(id==ID_BTN_CAR_IMG1 || id==ID_BTN_CAR_IMG2 || id==ID_BTN_OWNER_DOC || id==ID_BTN_BODY_DOC) g_GUI->drawCustomButton(dis, "Upload", RGB(71, 85, 105));
        else if(id==ID_BTN_DETAILS) g_GUI->drawCustomButton(dis, "View Details", ACCENT_CLR);
        else if(id==ID_BTN_PAY) g_GUI->drawCustomButton(dis, "Make Payment", GREEN_CLR);
        else if(id==ID_BTN_SEARCH) g_GUI->drawCustomButton(dis, "Search", ACCENT_CLR);
        else if(id==ID_BTN_RESET_SEARCH) g_GUI->drawCustomButton(dis, "Reset", RGB(100, 116, 139));
        // Clean Listing Cards
        else if(id==ID_LISTBOX && dis->itemID!=(UINT)-1){
            HDC dc=dis->hDC; RECT r=dis->rcItem;
            int lid=SendMessage(g_GUI->hwndList,LB_GETITEMDATA,dis->itemID,0);
            Listing* item=(Listing*)g_GUI->hashTable.search(lid);
            HBRUSH listBg=CreateSolidBrush(BG_CLR);
            RECT cardRect = {r.left+9, r.top+6, r.right-9, r.bottom-6};
            FillRect(dc,&r,listBg);
            DeleteObject(listBg);
            COLORREF border=(dis->itemState&ODS_SELECTED)?ACCENT_CLR:RGB(226, 232, 240);
            drawSoftShadow(dc,cardRect);
            drawRoundBox(dc,cardRect,RGB(255, 255, 255),border,12);
            
            COLORREF sc=ACCENT_CLR;
            if(item&&item->status==L_STATUS_PENDING) sc=ORANGE_CLR;
            else if(item&&(item->status==L_STATUS_SOLD||item->status==L_STATUS_RENTED)) sc=RED_CLR;
            else if(item&&item->status==L_STATUS_AVAILABLE) sc=GREEN_CLR;

            if(item){
                SetBkMode(dc,TRANSPARENT);
                string typeStr=(item->type==TYPE_HOUSE)?"HOUSE":"CAR";
                string stat=(item->status==L_STATUS_PENDING)?"PENDING":(item->status==L_STATUS_SOLD)?"SOLD":(item->status==L_STATUS_RENTED)?"RENTED":"AVAILABLE";
                string priceStr="ETB "+to_string_compat((int)item->price);
                
                RECT accent={cardRect.left,cardRect.top,cardRect.left+5,cardRect.bottom};
                HBRUSH abr=CreateSolidBrush(item->type==TYPE_HOUSE ? RGB(37, 99, 235) : RGB(124, 58, 237));
                FillRect(dc,&accent,abr); DeleteObject(abr);

                RECT tag = {cardRect.left+16, cardRect.top+12, cardRect.left+82, cardRect.top+33};
                drawRoundBox(dc,tag,item->type==TYPE_HOUSE ? RGB(219, 234, 254) : RGB(237, 233, 254),
                             item->type==TYPE_HOUSE ? RGB(191, 219, 254) : RGB(221, 214, 254),10);
                SetTextColor(dc, RGB(255,255,255)); SelectObject(dc, g_GUI->hFontSmall);
                SetTextColor(dc,item->type==TYPE_HOUSE ? RGB(30, 64, 175) : RGB(91, 33, 182));
                DrawText(dc, typeStr.c_str(), -1, &tag, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

                SelectObject(dc,g_GUI->hFontBold); SetTextColor(dc,RGB(15, 23, 42));
                RECT lr={cardRect.left+96,cardRect.top+10,cardRect.right-118,cardRect.top+36};
                DrawText(dc,priceStr.c_str(),-1,&lr,DT_LEFT|DT_SINGLELINE);
                
                SelectObject(dc,g_GUI->hFontNormal); SetTextColor(dc,RGB(100, 116, 139));
                RECT locr={cardRect.left+16,cardRect.top+42,cardRect.right-118,cardRect.top+66};
                string modelStr = (item->type==TYPE_CAR) ? ((Car*)item)->model + " - " : "";
                DrawText(dc,(modelStr + item->location).c_str(),-1,&locr,DT_LEFT|DT_SINGLELINE);

                SelectObject(dc,g_GUI->hFontSmall); SetTextColor(dc,RGB(148, 163, 184));
                RECT owner={cardRect.left+16,cardRect.top+67,cardRect.right-118,cardRect.bottom-10};
                string ownerStr="Seller: " + item->ownerUsername + "   ID: " + to_string_compat(item->id);
                DrawText(dc,ownerStr.c_str(),-1,&owner,DT_LEFT|DT_SINGLELINE);
                
                RECT br={cardRect.right-108,cardRect.top+32,cardRect.right-18,cardRect.top+56};
                drawRoundBox(dc,br,RGB(248,250,252),RGB(226,232,240),10);
                SelectObject(dc,g_GUI->hFontSmall); SetTextColor(dc, sc);
                DrawText(dc,stat.c_str(),-1,&br,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
            }
        }
        return TRUE;
    }
    case WM_MEASUREITEM: {
        ((LPMEASUREITEMSTRUCT)lParam)->itemHeight=CARD_H; return TRUE;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdc=(HDC)wParam; HWND ctrl=(HWND)lParam;
        SetBkMode(hdc,TRANSPARENT);
        if(ctrl==g_GUI->hDashTitle||ctrl==g_GUI->hUserWelcome){ 
            SetTextColor(hdc,RGB(248, 250, 252)); 
            return (LRESULT)g_GUI->hHeaderBrush; 
        }
        if(ctrl==g_GUI->hLoginTitle||ctrl==g_GUI->hLoginSubtitle||ctrl==g_GUI->hRegTitle||ctrl==g_GUI->hRegSubtitle){
            SetTextColor(hdc,(ctrl==g_GUI->hLoginTitle||ctrl==g_GUI->hRegTitle)?RGB(15, 23, 42):RGB(100, 116, 139));
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }
        if(ctrl==g_GUI->hDetailPanel){
            SetTextColor(hdc, RGB(71, 85, 105));
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }
        SetTextColor(hdc, RGB(71, 85, 105));
        return (LRESULT)g_GUI->hBgBrush;
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc=(HDC)wParam; SetBkColor(hdc,RGB(255,255,255)); SetTextColor(hdc,RGB(15, 23, 42));
        return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    case WM_CTLCOLORLISTBOX: {
        HDC hdc=(HDC)wParam; SetBkColor(hdc,BG_CLR); SetTextColor(hdc,RGB(15, 23, 42));
        return (LRESULT)g_GUI->hBgBrush;
    }
    case WM_COMMAND: {
        int id=LOWORD(wParam);
        switch(id){
            case ID_BTN_LOGIN:       g_GUI->attemptLogin(); break;
            case ID_BTN_LOGOUT:      g_GUI->logout(); break;
            case ID_BTN_GOTO_REG:    g_GUI->currentState=STATE_REGISTER; g_GUI->showRegister(); InvalidateRect(hwnd,NULL,TRUE); break;
            case ID_BTN_BACK_LOGIN:  g_GUI->currentState=STATE_LOGIN; g_GUI->showLogin(); InvalidateRect(hwnd,NULL,TRUE); break;
            case ID_BTN_REGISTER:    g_GUI->attemptRegister(); break;
            case ID_BTN_UPLOAD:      g_GUI->uploadDocument(); break;
            case ID_BTN_SEARCH:      g_GUI->applySearch(); break;
            case ID_BTN_RESET_SEARCH:g_GUI->clearSearch(); break;
            case ID_BTN_ADD:         g_GUI->addListing(); break;
            case ID_BTN_DELETE:      g_GUI->deleteSelectedListing(); break;
            case ID_BTN_BUY:
                if(g_GUI->currentUser->role == ROLE_SELLER) g_GUI->showDashboard();
                else g_GUI->buyListing();
                break;
            case ID_BTN_APPROVE:     g_GUI->approveListing(); break;
            case ID_BTN_PROCESS:     g_GUI->processOrder(); break;
            case ID_BTN_OPEN_DOC:    g_GUI->openSelectedDocument(); break;
            case ID_BTN_APPROVE_ACC: g_GUI->approveSelectedAccount(); break;
            case ID_BTN_REJECT_ACC:  g_GUI->rejectSelectedAccount(); break;
            case ID_BTN_CAR_IMG1:
            case ID_BTN_CAR_IMG2:
            case ID_BTN_OWNER_DOC:
            case ID_BTN_BODY_DOC:    g_GUI->uploadCarFile(id); break;
            case ID_BTN_DETAILS:     g_GUI->viewListingDetails(); break;
            case ID_BTN_PAY:         g_GUI->pay(); break;
            case ID_LISTBOX:
                if(HIWORD(wParam)==LBN_SELCHANGE){
                    int sel=SendMessage(g_GUI->hwndList,LB_GETCURSEL,0,0);
                    if(sel!=LB_ERR){
                        int lid=SendMessage(g_GUI->hwndList,LB_GETITEMDATA,sel,0);
                        Listing* item=(Listing*)g_GUI->hashTable.search(lid);
                        g_GUI->updateDetailPanel(item);
                    }
                }
                break;
            case ID_COMBO_TYPE:
                if(HIWORD(wParam)==CBN_SELCHANGE) g_GUI->showDashboard();
                break;
            case ID_BTN_SORT:
                Algorithms::bubbleSortByPrice(g_GUI->listings);
                g_GUI->populateList();
                MessageBox(hwnd,"Sorted by price!","Sorted",MB_OK|MB_ICONINFORMATION);
                break;
        }
        return 0;
    }
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}
