#pragma once
#include "IPlayerUI.h"
#include "MusicPlayer2.h"


class CMiniModeUI :
	public IPlayerUI
{
public:
	struct UIColors		//������ɫ
	{
		COLORREF color_text;				//�ı���ɫ
		COLORREF color_text_lable;			//��ǩ�ı�����ɫ
		COLORREF color_text_2;				//���δ�����ı�����ɫ
		//COLORREF color_text_heighlight;		//���ָ��ʱ�ı�����ɫ
		COLORREF color_back;				//������ɫ
		COLORREF color_lyric_back;			//��ʽ��汳����ɫ
		//COLORREF color_control_bar_back;	//������������ɫ
		COLORREF color_spectrum;			//Ƶ�׷������ε���ɫ
		//COLORREF color_spectrum_cover;		//��ר������ʱ��Ƶ�׷������ε���ɫ
		COLORREF color_spectrum_back;		//Ƶ�׷����ı�����ɫ
		COLORREF color_button_back;			//��ʷ��밴ť�ı���ɫ
		int background_transparency;		//������͸����0~100
	};

	struct SMiniModeUIData
	{
		int widnow_width;
		int window_height;
		int window_height2;
		int margin;
		bool m_show_volume{ false };	//����ָʾ�Ƿ�����ʾʱ��Ŀؼ���ʾ������������������ʱ��1.5���ڣ��˱�����ֵΪtrue

		CImage* pDefaultBackground = nullptr;			//Ĭ�ϵı���
		DisplayFormat* pDisplayFormat = nullptr;

		void Init()
		{
			widnow_width = theApp.DPI(304);
			window_height = theApp.DPI(44);
			window_height2 = theApp.DPI(336);
			margin = theApp.DPI(3);
		}
	};

public:
	CMiniModeUI(SMiniModeUIData& ui_data, CWnd* pMiniModeWnd);
	~CMiniModeUI();

	void SetToolTip(CToolTipCtrl* pToolTip);
	bool PointInControlArea(CPoint point) const;		//�ж�һ�����λ���Ƿ��ڿؼ�����

	virtual void Init(CDC* pDC) override;
	virtual void DrawInfo(bool reset = false) override;

	virtual void RButtonUp(CPoint point) override;
	virtual void MouseMove(CPoint point) override;
	virtual void LButtonUp(CPoint point) override;
	virtual void OnSizeRedraw(int cx, int cy) override;

	virtual CRect GetThumbnailClipArea() override;

private:
	void DrawUIButton(CRect rect, UIButton& btn, HICON icon, bool draw_background);
	void DrawTextButton(CRect rect, UIButton& btn, LPCTSTR text, bool draw_background);
	void AddMouseToolTip(const UIButton& btn, LPCTSTR str, bool* static_bool);		//Ϊһ����ť���������ʾ��ֻ������Ӧ��WM_MOUSEMOVE��ʱ����

private:
	SMiniModeUIData& m_ui_data;
	CWnd* m_pMiniModeWnd = nullptr;
	CDC* m_pDC;
	CDrawCommon m_draw;
	UIColors m_colors;
	CToolTipCtrl* m_tool_tip = nullptr;

	UIButton m_previous_btn;
	UIButton m_play_pause_btn;
	UIButton m_next_btn;
	UIButton m_playlist_btn;
	UIButton m_return_btn;
	UIButton m_close_btn;
};
