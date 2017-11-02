//-----------------------------------------------
//
//  忙しい人のためのゲームひな形 for Siv3D v1.0.1
//
//  This file is part of the Siv3D Engine.
//
//  Copyright (C) 2008-2016 Ryo Suzuki
//
//  Licensed under the MIT License.
//
//-----------------------------------------------

# include <Siv3D.hpp>
# include <HamFramework.hpp>

///////////////////////////////////////////////////////////////////////
//
//  ゲームの基本情報
//
namespace GameInfo
{
	// ゲームのタイトル
	const String Title = L"Biscuit Clicker";

	// ゲームのバージョン（空の文字列も OK）
	const String Version = L"Version 1.0";

	// ゲームの Web サイト（無い場合は空の文字列にする）
	const String WebURL = L"https://www.google.co.jp/";

	// 結果ツイートの文章（TweetHead + score + TweetTail)
	const String TweetHead = L"Biscuit Clicker をプレイしたよ。結果: ";

	const String TweetTail = L" 枚 #BiscuitClicker";

	// ゲームの背景色
	const ColorF BackgroundColor = ColorF(0.4, 0.7, 0.5);

	// シーン切り替え時のフェードイン・アウトの色
	const ColorF FadeInColor = ColorF(1.0, 1.0, 1.0);

	// タイトルのフォントサイズ
	const int32 TitleFontSize = 72;

	// メニューのフォントサイズ
	const int32 MenuFontSize = 24;

	// メニュー項目「ゲームスタート」
	const String MenuGameStart = L"ゲームスタート";

	// メニュー項目「スコア」
	const String MenuScore = L"スコア";

	// メニュー項目「クレジット」
	const String MenuCredit = L"クレジット";

	// メニュー項目「Web サイト」
	const String MenuWebURL = L"Web サイト";

	// メニュー項目「終了」
	const String MenuExit = L"終了";

	// セーブデータの保存場所
	const FilePath SaveFilePath = L"Save/Score.dat";

	// スタッフロールのクレジット（項目は増減できる）
	const Array<std::pair<String, Array<String>>> Credits
	{
		{ L"ゲームデザイン",{ L"Aaa Aaaa" } },
		{ L"プログラム",{ L"Bbb Bbbb", L"Cccc Cccccc" } },
		{ L"ゲームアート",{ L"Ddddd Ddddd" } },
		{ L"音楽",{ L"Eeeee Eeee" } },
		{ L"Special Thanks",{ L"You!" } },
	};
}

///////////////////////////////////////////////////////////////////////
//
//  タイトル画面の背景エフェクト
//
struct TitleBackGroundEffect : IEffect
{
	Line m_line;

	TitleBackGroundEffect()
	{
		const Vec2 pos = RandomVec2(Window::ClientRect());
		const Vec2 direction = Circular(Window::Width() + Window::Height(), Random(TwoPi));
		m_line.set(pos - direction, pos + direction);
	}

	bool update(double timeSec)
	{
		m_line.draw(2, AlphaF((1.0 - Abs(timeSec - 1.0)) * 0.3));
		return timeSec < 2.0;
	}
};

///////////////////////////////////////////////////////////////////////
//
//  タイトル画面のメニュー選択時のエフェクト
//
struct MenuEffect : IEffect
{
	Rect m_rect;

	MenuEffect(const Rect& rect)
		: m_rect(rect) {}

	bool update(double timeSec)
	{
		const double e = EaseOut<Easing::Quad>(timeSec);
		RectF(m_rect).stretched(e * 20).shearedX(20).draw(AlphaF((1.0 - e) * 0.4));
		return timeSec < 1.0;
	}
};

///////////////////////////////////////////////////////////////////////
//
//  ビスケットの描画
//
void DrawBiscuit(const Circle& c, bool drawShadow)
{
	c.drawShadow({ 0, 2 }, 12, 2 + drawShadow * 2)
		.draw(Color(250, 180, 100))
		.drawFrame(1, 1, Color(240, 140, 80));

	Circle(c.center, 1.5).draw(ColorF(0.0, 0.3));

	for (auto i : step(6))
	{
		Circle(c.center + Circular(c.r * 0.5, i * 60_deg), 1.5).draw(ColorF(0.0, 0.3));
	}
}

///////////////////////////////////////////////////////////////////////
//
//  全てのシーンからアクセスできるデータ
//
struct GameData
{
	int32 lastScore = 0;
};

using MyApp = SceneManager<String, GameData>;

///////////////////////////////////////////////////////////////////////
//
//  セーブ用スコアデータ
//
struct ScoreData
{
	int32 score;

	Date date;

	template <class Archive>
	void serialize(Archive& archive)
	{
		archive(score, date);
	}
};

///////////////////////////////////////////////////////////////////////
//
//  デフォルトのハイスコア
//
const std::array<ScoreData, 5> defaultHighScores
{
	ScoreData{ 50, Date(2017,1,1) },
	ScoreData{ 40, Date(2017,1,1) },
	ScoreData{ 30, Date(2017,1,1) },
	ScoreData{ 20, Date(2017,1,1) },
	ScoreData{ 10, Date(2017,1,1) },
};

///////////////////////////////////////////////////////////////////////
//
//  タイトル画面
//
class Title : public MyApp::Scene
{
private:

	Effect m_effect;

	Stopwatch m_effectBackgroundStopwatch{ true };

	Stopwatch m_effectMenuItemStopwatch{ true };

	Array<Rect> m_menuBoxes;

	Array<String> m_menuTexts =
	{
		GameInfo::MenuGameStart,
		GameInfo::MenuScore,
		GameInfo::MenuCredit,
		GameInfo::MenuWebURL,
		GameInfo::MenuExit
	};

public:

	~Title()
	{
		Cursor::SetStyle(CursorStyle::Default);
	}

	void init() override
	{
		if (GameInfo::WebURL.isEmpty)
		{
			m_menuTexts.erase(m_menuTexts.begin() + 3);
		}

		m_menuBoxes.resize(m_menuTexts.size());

		int32 boxWidth = 0;

		for (const auto& text : m_menuTexts)
		{
			boxWidth = Max(boxWidth, FontAsset(L"Menu")(text).region().w);
		}

		for (auto i : step(m_menuBoxes.size()))
		{
			m_menuBoxes[i].set(240, 280 + i * 80, boxWidth + 80, 60);
		}
	}

	void update() override
	{
		bool handCursor = false;

		for (auto i : step(m_menuBoxes.size()))
		{
			const Quad item = m_menuBoxes[i].shearedX(20);

			handCursor |= item.mouseOver;

			if (item.mouseOver && m_effectMenuItemStopwatch.elapsed() > 300ms)
			{
				m_effect.add<MenuEffect>(m_menuBoxes[i]);

				m_effectMenuItemStopwatch.restart();
			}

			if (item.leftClicked)
			{
				if (i == 0)
				{
					changeScene(L"Game");
				}
				else if (i == 1)
				{
					changeScene(L"Score");
				}
				else if (i == 2)
				{
					changeScene(L"Credit");
				}
				else if (!GameInfo::WebURL.isEmpty && i == 3)
				{
					Internet::LaunchWebBrowser(GameInfo::WebURL);
				}
				else
				{
					System::Exit();
				}

				break;
			}
		}

		if (m_effectBackgroundStopwatch.elapsed() > 50ms)
		{
			m_effect.add<TitleBackGroundEffect>();

			m_effectBackgroundStopwatch.restart();
		}

		Cursor::SetStyle(handCursor ? CursorStyle::Hand : CursorStyle::Default);
	}

	void draw() const override
	{
		Graphics2D::SetBlendState(BlendState::Additive);

		m_effect.update();

		Graphics2D::SetBlendState(BlendState::Default);

		const double titleHeight = FontAsset(L"Title")(GameInfo::Title).region().h;

		FontAsset(L"Title")(GameInfo::Title).drawAt(Window::Center().x, titleHeight);

		for (auto i : step(m_menuBoxes.size()))
		{
			m_menuBoxes[i].shearedX(20).draw();

			FontAsset(L"Menu")(m_menuTexts[i]).drawAt(m_menuBoxes[i].center, Color(40));
		}

		const Size versionSize = FontAsset(L"Version")(GameInfo::Version).region().size;

		FontAsset(L"Version")(GameInfo::Version).drawAt(Window::Size().moveBy(-versionSize));
	}
};

///////////////////////////////////////////////////////////////////////
//
//  ゲーム
//
class Game : public MyApp::Scene
{
private:

	static constexpr int32 gameTimeMillisec = 20 * 1000;

	Stopwatch m_countDownTimer;

	Stopwatch m_gameTimer;

	Circle m_biscuit;

	int32 m_score = 0;

	Circle generateBiscuit() const
	{
		return Circle(RandomVec2(RectF(Window::Size() * 0.8).setCenter(Window::Center())), Random(40, 60));
	}

	bool onCountDown() const
	{
		return m_countDownTimer.isActive() && m_countDownTimer.ms() < 4000;
	}

	bool onGame() const
	{
		return m_gameTimer.isActive();
	}

public:

	~Game()
	{
		Cursor::SetStyle(CursorStyle::Default);
	}

	void init() override
	{
		m_data->lastScore = 0;
	}

	void update() override
	{
		if (!onGame() && !m_countDownTimer.isActive())
		{
			m_countDownTimer.start();
		}

		if (!onGame() && m_countDownTimer.ms() >= 3000)
		{
			m_gameTimer.start();

			m_biscuit = generateBiscuit();
		}

		if (!onGame())
		{
			return;
		}

		if (m_gameTimer.ms() >= gameTimeMillisec)
		{
			m_data->lastScore = m_score;

			changeScene(L"Result", 2000);
		}

		const bool handCursor = m_biscuit.mouseOver;

		Cursor::SetStyle(handCursor ? CursorStyle::Hand : CursorStyle::Default);

		if (m_biscuit.leftClicked)
		{
			++m_score;

			m_biscuit = generateBiscuit();
		}
	}

	void draw() const override
	{
		if (onCountDown())
		{
			const int32 timeMillisec = Max((3999 - m_countDownTimer.ms()), 0);
			const int32 countDown = timeMillisec / 1000;
			const double e = EaseIn<Easing::Expo>((timeMillisec % 1000) / 1000.0);

			if (countDown > 0)
			{
				Transformer2D t(Mat3x2::Scale(1.0 + e * 2, Window::Center()));
				FontAsset(L"CountDown")(countDown).drawAt(Window::Center());
			}
			else
			{
				Transformer2D t(Mat3x2::Scale(1.0 + (1.0 - e) * 2, Window::Center()));
				FontAsset(L"CountDown")(L"START").drawAt(Window::Center(), AlphaF(e));
			}
		}

		if (!onGame())
		{
			return;
		}

		DrawBiscuit(m_biscuit, m_biscuit.mouseOver);

		const int32 timeLeftMillisec = Max(gameTimeMillisec - m_gameTimer.ms(), 0);

		FontAsset(L"GameTime")(L"TIME: {:0>2}'{:0>2}"_fmt, timeLeftMillisec / 1000, timeLeftMillisec % 1000 / 10).draw(60, 60);
	}
};

///////////////////////////////////////////////////////////////////////
//
//  結果画面
//
class Result : public MyApp::Scene
{
private:

	std::array<ScoreData, 5> m_highScores = defaultHighScores;

	const Circle titleButton = Circle(Window::Center().x - 300, Window::Height() * 0.7, 35);

	const Circle tweetButton = Circle(Window::Center().x + 300, Window::Height() * 0.7, 35);

public:

	~Result()
	{
		Cursor::SetStyle(CursorStyle::Default);
	}

	void init() override
	{
		if (FileSystem::Exists(GameInfo::SaveFilePath))
		{
			Deserializer<BinaryReader>{GameInfo::SaveFilePath}(m_highScores);
		}
		else
		{
			Serializer<BinaryWriter>{GameInfo::SaveFilePath}(m_highScores);
		}

		if (m_highScores.back().score <= m_data->lastScore)
		{
			m_highScores.back() = { m_data->lastScore, Date::Today() };

			std::sort(m_highScores.begin(), m_highScores.end(), [](const ScoreData& a, const ScoreData& b)
			{
				return a.score > b.score ? true : a.score == b.score ? a.date > b.date : false;
			});

			Serializer<BinaryWriter>{GameInfo::SaveFilePath}(m_highScores);
		}
	}

	void update() override
	{
		if (titleButton.leftClicked || Input::KeyEscape.clicked)
		{
			changeScene(L"Title");
		}

		if (tweetButton.leftClicked)
		{
			const String tweetMessage = Format(GameInfo::TweetHead, m_data->lastScore, GameInfo::TweetTail);

			Twitter::OpenTweetWindow(tweetMessage);
		}

		const bool handCursor = titleButton.mouseOver || tweetButton.mouseOver;

		Cursor::SetStyle(handCursor ? CursorStyle::Hand : CursorStyle::Default);
	}

	void draw() const override
	{
		DrawBiscuit(Circle(Window::Center().x - 80, Window::Height() * 0.4, 60), false);

		const double resultHeight = FontAsset(L"Result")(L"x", m_data->lastScore).region().h;

		FontAsset(L"Result")(L"x", m_data->lastScore).draw(Window::Center().x + 50, Window::Height() * 0.4 - resultHeight / 2);

		DrawBiscuit(titleButton, titleButton.mouseOver);
		FontAsset(L"ResultButton")(L"タイトルへ").drawAt(titleButton.center.movedBy(0, 90));

		DrawBiscuit(tweetButton, tweetButton.mouseOver);
		FontAsset(L"ResultButton")(L"結果をツイート").drawAt(tweetButton.center.movedBy(0, 90));
	}
};

///////////////////////////////////////////////////////////////////////
//
//  ハイスコア一覧
//
class Score : public MyApp::Scene
{
private:

	std::array<ScoreData, 5> m_highScores = defaultHighScores;

public:

	void init() override
	{
		if (FileSystem::Exists(GameInfo::SaveFilePath))
		{
			Deserializer<BinaryReader>{GameInfo::SaveFilePath}(m_highScores);
		}
		else
		{
			Serializer<BinaryWriter>{GameInfo::SaveFilePath}(m_highScores);
		}
	}

	void update() override
	{
		if ((Input::MouseL | Input::KeyEscape).clicked)
		{
			changeScene(L"Title");
		}
	}

	void draw() const override
	{
		const int32 h = FontAsset(L"ScoreList").height;

		for (auto i : step(m_highScores.size()))
		{
			const Rect rect = Rect(520, 100).setCenter(Window::Center().x, 120 + i * 120);

			rect.draw(ColorF(1.0, 0.2));

			FontAsset(L"ScoreList")(m_highScores[i].score)
				.draw(rect.pos + Point(42, (rect.h - h) / 2 + 2), Palette::Gray);

			FontAsset(L"ScoreList")(m_highScores[i].score)
				.draw(rect.pos + Point(40, (rect.h - h) / 2));

			const Size dateSize = FontAsset(L"ScoreListDate")(m_highScores[i].date).region().size;

			FontAsset(L"ScoreListDate")(m_highScores[i].date)
				.draw(rect.br.x - dateSize.x - 40, rect.center.y - dateSize.y / 2);

			for (auto k : step(5 - i))
			{
				const Point left(rect.center.movedBy(-rect.w / 2 - 40 - k * 50, 0));
				const Point right(rect.center.movedBy(rect.w / 2 + 40 + k * 50, 0));

				Rect(40).setCenter(left).rotated(45_deg).draw(AlphaF(1.0 - i * 0.1));
				Rect(40).setCenter(right).rotated(45_deg).draw(AlphaF(1.0 - i * 0.1));
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////
//
//  スタッフクレジット
//
class Credit : public MyApp::Scene
{
private:

	Array<std::tuple<String, double, bool>> m_credits;

	double m_height = 0.0;

	Stopwatch m_stopwatch{ true };

	double yOffset() const
	{
		return Window::Height() + 60 - m_stopwatch.ms() / 20.0;
	}

public:

	void init()
	{
		double y = 0;

		for (const auto& credit : GameInfo::Credits)
		{
			m_credits.emplace_back(credit.first, y, true);
			y += 70;

			for (const auto& name : credit.second)
			{
				m_credits.emplace_back(name, y, false);
				y += 60;
			}

			y += 60;
		}

		m_height = y;
	}

	void update() override
	{
		if ((Input::MouseL | Input::KeyEscape).clicked)
		{
			changeScene(L"Title");
		}

		if ((m_height + yOffset()) < 0)
		{
			m_stopwatch.restart();
		}
	}

	void draw() const override
	{
		const double offset = yOffset();

		for (const auto& credit : m_credits)
		{
			FontAsset(std::get<bool>(credit) ? L"CreditBig" : L"CreditSmall")(std::get<String>(credit))
				.drawAt(Window::Center().x, std::get<double>(credit) + offset);
		}
	}
};

///////////////////////////////////////////////////////////////////////
//
//  メイン関数
//
void Main()
{
	Window::Resize(1280, 720);
	Window::SetTitle(GameInfo::Title);
	Graphics::SetBackground(GameInfo::BackgroundColor);
	System::SetExitEvent(WindowEvent::CloseButton);

	//////////////////////////////////////////////////////////////
	//
	//  使用するシーン
	//
	MyApp manager;
	manager.setFadeColor(GameInfo::FadeInColor);
	manager.add<Title>(L"Title");
	manager.add<Game>(L"Game");
	manager.add<Result>(L"Result");
	manager.add<Score>(L"Score");
	manager.add<Credit>(L"Credit");

	//////////////////////////////////////////////////////////////
	//
	//  使用するフォント
	//
	FontAsset::Register(L"Title", GameInfo::TitleFontSize, Typeface::Heavy, FontStyle::Outline);
	FontAsset(L"Title").changeOutlineStyle(TextOutlineStyle(Color(60), Color(255), GameInfo::TitleFontSize * 0.05));
	FontAsset::Register(L"Menu", GameInfo::MenuFontSize, Typeface::Bold);
	FontAsset::Register(L"Version", 14, Typeface::Regular);
	FontAsset::Register(L"CountDown", 72, Typeface::Bold);
	FontAsset::Register(L"Result", 80, Typeface::Bold);
	FontAsset::Register(L"ResultButton", 32, Typeface::Regular);
	FontAsset::Register(L"GameTime", 40, Typeface::Light);
	FontAsset::Register(L"ScoreList", 50, Typeface::Heavy);
	FontAsset::Register(L"ScoreListDate", 25, Typeface::Regular, FontStyle::Italic);
	FontAsset::Register(L"CreditBig", 32, Typeface::Bold);
	FontAsset::Register(L"CreditSmall", 28, Typeface::Regular);

	//////////////////////////////////////////////////////////////
	//
	//  メインループ
	//
	while (System::Update())
	{
		if (!manager.updateAndDraw())
		{
			break;
		}
	}
}