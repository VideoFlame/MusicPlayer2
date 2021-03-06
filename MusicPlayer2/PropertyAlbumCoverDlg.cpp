﻿// PropertyAlbumCover.cpp: 实现文件
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "PropertyAlbumCoverDlg.h"
#include "COSUPlayerHelper.h"
#include "TagLabHelper.h"
#include "DrawCommon.h"

#define PROPERTY_COVER_IMG_FILE_NAME L"PropertyCoverImg_U6V19HODcJ2p11FM"

// CPropertyAlbumCoverDlg 对话框

IMPLEMENT_DYNAMIC(CPropertyAlbumCoverDlg, CTabDlg)

CPropertyAlbumCoverDlg::CPropertyAlbumCoverDlg(vector<SongInfo>& all_song_info, int& index, bool read_only /*= false*/, CWnd* pParent /*=nullptr*/)
	: CTabDlg(IDD_PROPERTY_ALBUM_COVER_DIALOG, pParent), m_all_song_info{ all_song_info }, m_index{ index }, m_read_only{ read_only }
{

}

CPropertyAlbumCoverDlg::~CPropertyAlbumCoverDlg()
{
}

void CPropertyAlbumCoverDlg::PagePrevious()
{
    m_modified = false;
    m_cover_changed = false;
    m_cover_deleted = false;
    m_index--;
    if (m_index < 0) m_index = static_cast<int>(m_all_song_info.size()) - 1;
    if (m_index < 0) m_index = 0;
    SetWreteEnable();
    ShowInfo();
}

void CPropertyAlbumCoverDlg::PageNext()
{
    m_modified = false;
    m_cover_changed = false;
    m_cover_deleted = false;
    m_index++;
    if (m_index >= static_cast<int>(m_all_song_info.size())) m_index = 0;
    SetWreteEnable();
    ShowInfo();
}

void CPropertyAlbumCoverDlg::SaveModified()
{
    int current_position{};
    bool is_playing{};
    if (IsCurrentSong())
    {
        current_position = CPlayer::GetInstance().GetCurrentPosition();
        is_playing = CPlayer::GetInstance().IsPlaying();
        CPlayer::GetInstance().MusicControl(Command::CLOSE);
    }

    if (m_cover_deleted)
    {
        CTagLabHelper::WriteAlbumCover(CurrentSong().file_path, wstring());
    }
    if (m_cover_changed)
    {
        CTagLabHelper::WriteAlbumCover(CurrentSong().file_path, m_out_img_path);
    }
    else if (IsDlgButtonChecked(IDC_SAVE_ALBUM_COVER_BUTTON) && !CPlayer::GetInstance().IsInnerCover() && CPlayer::GetInstance().AlbumCoverExist())
    {
        CTagLabHelper::WriteAlbumCover(CurrentSong().file_path, CPlayer::GetInstance().GetAlbumCoverPath());
    }

    if (IsCurrentSong())
    {
        CPlayer::GetInstance().MusicControl(Command::OPEN);
        CPlayer::GetInstance().SeekTo(current_position);
        if (is_playing)
            CPlayer::GetInstance().MusicControl(Command::PLAY);
    }
}

void CPropertyAlbumCoverDlg::AdjustColumnWidth()
{
    CRect rect;
    m_list_ctrl.GetWindowRect(rect);
    int width0 = theApp.DPI(85);
    int width1 = rect.Width() - width0 - theApp.DPI(20) - 1;
    m_list_ctrl.SetColumnWidth(0, width0);
    m_list_ctrl.SetColumnWidth(1, width1);
}

void CPropertyAlbumCoverDlg::ShowInfo()
{
    //载入封面图片
    m_cover_img.Destroy();
    int cover_type{};
    size_t cover_size{};
    if (m_cover_changed)
    {
        m_cover_img.Load(m_out_img_path.c_str());
        cover_size = CCommon::GetFileSize(m_out_img_path);
    }
    else if (!IsCurrentSong())
    {
        wstring file_path = CurrentSong().file_path;
        wstring file_ext = CFilePathHelper(file_path).GetFileExtension();
        //获取专辑封面
        string img_data;
        if (file_ext == L"mp3")
            img_data = CTagLabHelper::GetMp3AlbumCover(file_path, cover_type);
        else if (file_ext == L"flac")
            img_data = CTagLabHelper::GetFlacAlbumCover(file_path, cover_type);
        else if (file_ext == L"m4a")
            img_data = CTagLabHelper::GetM4aAlbumCover(file_path, cover_type);
        cover_size = img_data.size();

        //将封面保存到临时目录
        if (!img_data.empty())
        {
            wstring cover_img_path = CCommon::GetTemplatePath() + PROPERTY_COVER_IMG_FILE_NAME;
            ofstream out_put{ cover_img_path, std::ios::binary };
            out_put << img_data;
            out_put.close();
            //载入图片
            m_cover_img.Load(cover_img_path.c_str());
        }
    }

    //显示列表信息
    //文件路径
    m_list_ctrl.SetItemText(RI_FILE_PATH, 1, CurrentSong().file_path.c_str());
    if (HasAlbumCover())
    {
        //路径
        CString str_path;
        if (m_cover_changed)
            str_path = m_out_img_path.c_str();
        else if (IsCurrentSong() && !CPlayer::GetInstance().IsInnerCover())
            str_path = CPlayer::GetInstance().GetAlbumCoverPath().c_str();
        else
            str_path = CCommon::LoadText(_T("<"), IDS_INNER_ALBUM_COVER, L">");
        m_list_ctrl.SetItemText(RI_COVER_PATH, 1, str_path);

        //文件类型
        CString str_type;
        if (m_cover_changed)
            str_type = CFilePathHelper(m_out_img_path).GetFileExtension().c_str();
        else if (IsCurrentSong() && !CPlayer::GetInstance().IsInnerCover())
            str_type = CFilePathHelper(CPlayer::GetInstance().GetAlbumCoverPath()).GetFileExtension().c_str();
        else
        {
            if (IsCurrentSong())
                cover_type = CPlayer::GetInstance().GetAlbumCoverType();
            switch (cover_type)
            {
            case 0:
                str_type = L"jpg";
                break;
            case 1:
                str_type = L"png";
                break;
            case 2:
                str_type = L"gif";
                break;
            case 3:
                str_type = L"bmp";
                break;
            default:
                break;
            }
        }
        m_list_ctrl.SetItemText(RI_FORMAT, 1, str_type);

        //宽度、高度、BPP
        int cover_width{};
        int cover_height{};
        int cover_bpp;
        if (IsCurrentSong() && !m_cover_changed)
        {
            cover_width = CPlayer::GetInstance().GetAlbumCoverInfo().width;
            cover_height = CPlayer::GetInstance().GetAlbumCoverInfo().height;
            cover_bpp = CPlayer::GetInstance().GetAlbumCoverInfo().bpp;
        }
        else
        {
            cover_width = m_cover_img.GetWidth();
            cover_height = m_cover_img.GetHeight();
            cover_bpp = m_cover_img.GetBPP();
        }
        m_list_ctrl.SetItemText(RI_WIDTH, 1, std::to_wstring(cover_width).c_str());
        m_list_ctrl.SetItemText(RI_HEIGHT, 1, std::to_wstring(cover_height).c_str());
        m_list_ctrl.SetItemText(RI_BPP, 1, std::to_wstring(cover_bpp).c_str());

        //文件大小
        if (IsCurrentSong() && !m_cover_changed)
        {
            size_t file_size = CCommon::GetFileSize(CPlayer::GetInstance().GetAlbumCoverPath());
            m_list_ctrl.SetItemText(RI_SIZE, 1, CCommon::DataSizeToString(file_size));
        }
        else
        {
            m_list_ctrl.SetItemText(RI_SIZE, 1, CCommon::DataSizeToString(cover_size));
        }

        //已压缩尺寸过大的专辑封面
        if (IsCurrentSong() && !m_cover_changed)
        {
            m_list_ctrl.SetItemText(RI_COMPRESSED, 0, CCommon::LoadText(IDS_ALBUM_COVER_COMPRESSED));
            m_list_ctrl.SetItemText(RI_COMPRESSED, 1, (CPlayer::GetInstance().GetAlbumCoverInfo().size_exceed ? CCommon::LoadText(IDS_YES) : CCommon::LoadText(IDS_NO)));
        }
        else
        {
            m_list_ctrl.SetItemText(RI_COMPRESSED, 0, _T(""));
            m_list_ctrl.SetItemText(RI_COMPRESSED, 1, _T(""));
        }
    }
    else
    {
        m_list_ctrl.SetItemText(RI_COMPRESSED, 0, _T(""));
        for (int i = 0; i < RI_MAX; i++)
        {
            if (i!=RI_FILE_PATH)
                m_list_ctrl.SetItemText(i, 1, _T(""));
        }
    }


    SetWreteEnable();
    CheckDlgButton(IDC_SAVE_ALBUM_COVER_BUTTON, FALSE);
    Invalidate();
}

const SongInfo& CPropertyAlbumCoverDlg::CurrentSong()
{
    if (m_index >= 0 && m_index < static_cast<int>(m_all_song_info.size()))
    {
        return m_all_song_info[m_index];
    }
    else
    {
        static SongInfo song;
        return song;
    }
}

CImage& CPropertyAlbumCoverDlg::GetCoverImage()
{
    if (IsCurrentSong() && !m_cover_changed)
        return CPlayer::GetInstance().GetAlbumCover();
    else
        return m_cover_img;
}

bool CPropertyAlbumCoverDlg::IsCurrentSong()
{
    const SongInfo song{ CurrentSong() };
    bool is_current_song{ song.file_path == CPlayer::GetInstance().GetCurrentFilePath() };
    return is_current_song;
}

bool CPropertyAlbumCoverDlg::HasAlbumCover()
{
    if (IsCurrentSong() && !m_cover_changed)
        return CPlayer::GetInstance().AlbumCoverExist();
    else
        return !m_cover_img.IsNull();
}

void CPropertyAlbumCoverDlg::SetWreteEnable()
{
    CFilePathHelper file_path{ m_all_song_info[m_index].file_path };
    m_write_enable = (!m_all_song_info[m_index].is_cue && !COSUPlayerHelper::IsOsuFile(file_path.GetFilePath()) && CTagLabHelper::IsFileTypeCoverWriteSupport(file_path.GetFileExtension()));
    EnableControls();
    SetSaveBtnEnable();
}

void CPropertyAlbumCoverDlg::EnableControls()
{

    EnableDlgCtrl(IDC_BROWSE_BUTTON, m_write_enable);
    EnableDlgCtrl(IDC_DELETE_BUTTON, m_write_enable && (!IsCurrentSong() || CPlayer::GetInstance().IsInnerCover()) && HasAlbumCover());
    EnableDlgCtrl(IDC_SAVE_ALBUM_COVER_BUTTON, m_write_enable && IsCurrentSong() && !CPlayer::GetInstance().IsInnerCover() && CPlayer::GetInstance().AlbumCoverExist());
    ShowDlgCtrl(IDC_SAVE_ALBUM_COVER_BUTTON, IsCurrentSong());
}

void CPropertyAlbumCoverDlg::SetSaveBtnEnable()
{
    bool enable = m_write_enable && (m_modified || m_cover_changed || m_cover_deleted);
    CWnd* pParent = GetParentWindow();
    if (pParent != nullptr)
        pParent->SendMessage(WM_PROPERTY_DIALOG_MODIFIED, enable);
}

void CPropertyAlbumCoverDlg::OnTabEntered()
{
    ShowInfo();
}

void CPropertyAlbumCoverDlg::DoDataExchange(CDataExchange* pDX)
{
    CTabDlg::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_list_ctrl);
}


BEGIN_MESSAGE_MAP(CPropertyAlbumCoverDlg, CTabDlg)
    ON_WM_MOUSEWHEEL()
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_SAVE_ALBUM_COVER_BUTTON, &CPropertyAlbumCoverDlg::OnBnClickedSaveAlbumCoverButton)
    ON_BN_CLICKED(IDC_DELETE_BUTTON, &CPropertyAlbumCoverDlg::OnBnClickedDeleteButton)
    ON_BN_CLICKED(IDC_BROWSE_BUTTON, &CPropertyAlbumCoverDlg::OnBnClickedBrowseButton)
END_MESSAGE_MAP()


// CPropertyAlbumCoverDlg 消息处理程序


BOOL CPropertyAlbumCoverDlg::OnInitDialog()
{
    CTabDlg::OnInitDialog();

    // TODO:  在此添加额外的初始化

    //初始化列表
    //初始化列表
    CRect rect;
    m_list_ctrl.GetWindowRect(rect);
    m_list_ctrl.SetExtendedStyle(m_list_ctrl.GetExtendedStyle() | LVS_EX_GRIDLINES);
    int width0 = theApp.DPI(85);
    int width1 = rect.Width() - width0 - theApp.DPI(20) - 1;
    m_list_ctrl.InsertColumn(0, CCommon::LoadText(IDS_ITEM), LVCFMT_LEFT, width0);
    m_list_ctrl.InsertColumn(1, CCommon::LoadText(IDS_VLAUE), LVCFMT_LEFT, width1);


    m_list_ctrl.InsertItem(RI_FILE_PATH, CCommon::LoadText(IDS_FILE_PATH));    //文件路径
    m_list_ctrl.InsertItem(RI_COVER_PATH, CCommon::LoadText(IDS_PATH));    //封面路径
    m_list_ctrl.InsertItem(RI_FORMAT, CCommon::LoadText(IDS_FORMAT));    //封面类型
    m_list_ctrl.InsertItem(RI_WIDTH, CCommon::LoadText(IDS_WIDTH));    //宽度
    m_list_ctrl.InsertItem(RI_HEIGHT, CCommon::LoadText(IDS_HEIGHT));    //高度
    m_list_ctrl.InsertItem(RI_BPP, CCommon::LoadText(IDS_BPP));    //每像素位数
    m_list_ctrl.InsertItem(RI_SIZE, CCommon::LoadText(IDS_FILE_SIZE));    //文件大小
    m_list_ctrl.InsertItem(RI_COMPRESSED, CCommon::LoadText(IDS_ALBUM_COVER_COMPRESSED));    //已压缩尺寸过大的专辑封面

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}


BOOL CPropertyAlbumCoverDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    //return CTabDlg::OnMouseWheel(nFlags, zDelta, pt);
    return TRUE;
}


void CPropertyAlbumCoverDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting
                       // TODO: 在此处添加消息处理程序代码
                       // 不为绘图消息调用 CTabDlg::OnPaint()

    CRect rect;
    GetClientRect(rect);

    CRect rect_list;
    m_list_ctrl.GetWindowRect(rect_list);
    ScreenToClient(rect_list);

    rect.right = rect_list.left;
    rect.DeflateRect(theApp.DPI(16), theApp.DPI(16));
    if (HasAlbumCover())
    {

        CImage& img{ GetCoverImage() };
        if (img.GetWidth() < rect.Width() && img.GetHeight() < rect.Height())
        {
            CRect rect_img;
            rect_img.left = rect.left + (rect.Width() - img.GetWidth()) / 2;
            rect_img.top = rect.top + (rect.Height() - img.GetHeight()) / 2;
            rect_img.right = rect_img.left + img.GetWidth();
            rect_img.bottom = rect_img.top + img.GetHeight();
            rect = rect_img;
        }
        CDrawCommon draw;
        draw.Create(&dc);
        draw.DrawImage(img, rect.TopLeft(), rect.Size(), CDrawCommon::StretchMode::FIT);
    }
    else
    {
        dc.FillSolidRect(rect, RGB(210, 210, 210));
    }

    int a = 0;
}


void CPropertyAlbumCoverDlg::OnBnClickedSaveAlbumCoverButton()
{
    // TODO: 在此添加控件通知处理程序代码
    m_modified = true;
    SetSaveBtnEnable();
}


void CPropertyAlbumCoverDlg::OnBnClickedDeleteButton()
{
    // TODO: 在此添加控件通知处理程序代码
    m_cover_deleted = true;
    SetSaveBtnEnable();
}


void CPropertyAlbumCoverDlg::OnBnClickedBrowseButton()
{
    // TODO: 在此添加控件通知处理程序代码

    CString filter = CCommon::LoadText(IDS_IMAGE_FILE_FILTER);
    CFileDialog fileDlg(TRUE, NULL, NULL, 0, filter, this);
    if (IDOK == fileDlg.DoModal())
    {
        m_out_img_path = fileDlg.GetPathName().GetString();
        m_cover_changed = true;
        ShowInfo();
    }

}
