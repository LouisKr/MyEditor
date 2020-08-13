#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

//allgemein 
int info_size = 24;
int offset_x = 40;
int offset_y = 30;
int line_space_offset = 10;
int line_offset_x = 0;
int line_offset_y = 0;
int y_changed = 0;
std::vector<std::string> strings;

//mark markieren
bool mark_all = false;

//save
std::ofstream myfile;
std::string openfile = "auto_generated.cpp";
//save_animation
bool save_animation = false;
int save_animation_step = 30;
int save_animation_timer = 1;
int save_animation_wait = 60;


//dialogbox
//box for file open and save etc
bool dialogbox_open = false;
int dialogbox_size = 100;
std::string dialogbox_txt = openfile;

//font
int font_size = 1;
int font_width = 8;
int font_height = 7;

//pacman animation
olc::Sprite pacman_sprite = olc::Sprite::Sprite("content/small_pecman.png");
int pacman_pos = 0;
int pacman_timer = 0;

void compile(std::string command)
{
	std::system(command.c_str());
	Sleep(400);
	system("auto_generated.exe");
}

#pragma region Zwischenablage 
class RaiiClipboard
{
public:
	RaiiClipboard()
	{
		if (!OpenClipboard(nullptr))
			throw std::runtime_error("Can't open clipboard.");
		// ... or define some custom exception class for clipboard errors.
	}

	~RaiiClipboard()
	{
		CloseClipboard();
	}

	// Ban copy   
private:
	RaiiClipboard(const RaiiClipboard&);
	RaiiClipboard& operator=(const RaiiClipboard&);
};

class RaiiTextGlobalLock
{
public:
	explicit RaiiTextGlobalLock(HANDLE hData)
		: m_hData(hData)
	{
		m_psz = static_cast<const char*>(GlobalLock(m_hData));
		if (!m_psz)
			throw std::runtime_error("Can't acquire lock on clipboard text.");
	}

	~RaiiTextGlobalLock()
	{
		GlobalUnlock(m_hData);
	}

	const char* Get() const
	{
		return m_psz;
	}

private:
	HANDLE m_hData;
	const char* m_psz;

	// Ban copy
	RaiiTextGlobalLock(const RaiiTextGlobalLock&);
	RaiiTextGlobalLock& operator=(const RaiiTextGlobalLock&);
};

std::string GetClipboardText()
{
	RaiiClipboard clipboard;

	HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == nullptr)
		throw std::runtime_error("Can't get clipboard text.");

	RaiiTextGlobalLock textGlobalLock(hData);
	std::string text(textGlobalLock.Get());

	return text;
}
#pragma endregion

std::string blue_words[] = { "int","char", "bool","float", "double", "void" , "if", "else" , "for" , "auto" , "const", "public" , "private" , "class" , "struct" , "enum" };
std::string orange_words[] = { "string","vector" , "std::string" };
std::string honeydew_words[] = { "#include","#define" };
char dark_orange_symbols[] = { '=', ';' , ',' , '{','}' , '.', '%', '(', ')' , '<' , '>' , '[' , ']' , ':' , '+','-','*', '/' };
char fuchsia_words[] = { '0','1','2','3','4','5','6','7','8','9' };
enum color { grey, blue, orange, dark_orange, fuchsia, honeydew };

void increase_font_size()
{
	font_size += 1;
	font_width += 8;
	font_height += 7;
}
void decrease_font_size()
{
	font_size -= 1;
	font_width -= 8;
	font_height -= 7;
}


//mouse_pointer
std::pair<int, int> mouse_position = { 0,0 };
int mouse_timer = 30;
bool show_mouse = true;

int width = 0;

void add_char(char character)
{
	if (dialogbox_open == false)
	{
		std::string temp = "";
		if (strings.size() <= mouse_position.second)
		{
			uint32_t needed_lines = mouse_position.second - strings.size() + 1;
			for (size_t i = 0; i < needed_lines; i++)
			{
				strings.push_back("");
			}
		}

		if (mouse_position.first > strings[mouse_position.second].size())
		{
			for (size_t i = 0; i < mouse_position.first; i++)
			{
				if (i < strings[mouse_position.second].size())
				{
					temp += strings[mouse_position.second][i];
				}
				else
				{
					temp += " ";
				}
			}
			temp += character;
		}
		else
		{
			for (size_t i = 0; i < strings[mouse_position.second].size() + 1; i++)
			{
				if (i == mouse_position.first)
				{
					temp += character;
				}
				temp += strings[mouse_position.second][i];
			}
		}

		if (mouse_position.first + (line_offset_x / font_width) > ((width - offset_x) / font_width) - 2)
		{
			line_offset_x -= font_width;
		}

		strings[mouse_position.second] = temp;
	}
	else
	{
		dialogbox_txt += character;
	}
}

void add_char(int position, char character)
{
	std::string temp = "";
	bool placed = false;
	for (size_t i = 0; i < strings[mouse_position.second].size() + 1; i++)
	{
		if (i == position)
		{
			temp += character;
			placed = true;
		}
		else
		{
			if (placed == false)
				temp += strings[mouse_position.second][i];
			else
				temp += strings[mouse_position.second][i - 1];
		}
	}
	strings[mouse_position.second] = temp;
}

//split big string in to words
std::vector<std::string> split_in_to_words(int pos)
{
	std::vector<std::string> words;
	std::string current_word = "";
	for (size_t i = 0; i < strings[pos].size(); i++)
	{
		char current_char = strings[pos][i];
		if (current_char != ' ' and current_char != '\0' /*and current_char != '\t'*/ and current_char != '\r' and current_char != '\n')
		{
			current_word += current_char;
		}
		else
		{
			words.push_back(current_word);
			current_word = "";
		}
	}
	words.push_back(current_word);
	return words;
}

void format()
{
	std::vector<std::string> temp;
	for (size_t i = 0; i < strings.size(); i++)
	{
		std::string new_line = "";
		for (size_t p = 0; p < strings[i].size(); p++)
		{
			if (strings[i][p] == '\t')
			{
				new_line += ' ';
				new_line += ' ';
			}
			else
			{
				if (strings[i][p] != '\0' && strings[i][p] != '\n' && strings[i][p] != '\r')
				{
					new_line += strings[i][p];
				}
			}
		}
		temp.push_back(new_line);
	}
	strings = temp;
}

class Editor : public olc::PixelGameEngine
{
	std::map<olc::Key, char> charKeys;
public:
	Editor()
	{
		sAppName = "Editor";

#pragma region key_mapping 
		charKeys[olc::Key::NONE] = 0x00;
		charKeys[olc::Key::A] = 'a';
		charKeys[olc::Key::B] = 'b';
		charKeys[olc::Key::C] = 'c';
		charKeys[olc::Key::D] = 'd';
		charKeys[olc::Key::E] = 'e';
		charKeys[olc::Key::F] = 'f';
		charKeys[olc::Key::G] = 'g';
		charKeys[olc::Key::H] = 'h';
		charKeys[olc::Key::I] = 'i';
		charKeys[olc::Key::J] = 'j';
		charKeys[olc::Key::K] = 'k';
		charKeys[olc::Key::L] = 'l';
		charKeys[olc::Key::M] = 'm';
		charKeys[olc::Key::N] = 'n';
		charKeys[olc::Key::O] = 'o';
		charKeys[olc::Key::P] = 'p';
		charKeys[olc::Key::Q] = 'q';
		charKeys[olc::Key::R] = 'r';
		charKeys[olc::Key::S] = 's';
		charKeys[olc::Key::T] = 't';
		charKeys[olc::Key::U] = 'u';
		charKeys[olc::Key::V] = 'v';
		charKeys[olc::Key::W] = 'w';
		charKeys[olc::Key::X] = 'x';
		charKeys[olc::Key::Y] = 'y';
		charKeys[olc::Key::Z] = 'z';
		charKeys[olc::Key::K0] = '0';
		charKeys[olc::Key::K1] = '1';
		charKeys[olc::Key::K2] = '2';
		charKeys[olc::Key::K3] = '3';
		charKeys[olc::Key::K4] = '4';
		charKeys[olc::Key::K5] = '5';
		charKeys[olc::Key::K6] = '6';
		charKeys[olc::Key::K7] = '7';
		charKeys[olc::Key::K8] = '8';
		charKeys[olc::Key::K9] = '9';
#pragma endregion 
	}

public:

	bool OnUserCreate() override
	{
		width = ScreenWidth();
		increase_font_size();

		std::string line;
		std::ifstream myfile("last_opend_file.txt");
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				openfile = line;
				dialogbox_txt = line;
			}
			myfile.close();
		}
		else
		{
			openfile = "auto_generated.cpp";
		}

		line = "";
		myfile.open(openfile);
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				strings.push_back(line);
			}
			myfile.close();
		}
		else
		{
			strings.push_back("Unable to open file");
			openfile = "unnamed";
		}

		/*strings.push_back("#include <iostream>");
		strings.push_back("int main()");
		strings.push_back("{");
		strings.push_back("std::cout << !Hello World!;");
		strings.push_back("}");*/

		mouse_position.first = 0;
		mouse_position.second = 0;
		return true;
	}


	bool OnUserDestroy() override
	{
		myfile.open("last_opend_file.txt");
		myfile << openfile;
		myfile.close();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);
		if (mark_all == true)
		{
			SetPixelMode(olc::Pixel::MASK);
			if (dialogbox_open == false)
				FillRect(offset_x, offset_y, ScreenWidth(), ScreenHeight(), olc::GREEN);
			else
				FillRect(0, info_size, ScreenWidth(), 100, olc::GREEN);
		}


		//mouse
		if (show_mouse == true)
		{
			//draw Mouse
			if (dialogbox_open == false)
			{
				FillRect((offset_x + line_offset_x) + (font_width * mouse_position.first), offset_y + (mouse_position.second * (font_height + line_space_offset) + line_offset_y) + font_height, font_width, font_height / 4, olc::WHITE);
			}

			if (mouse_timer == 0)
				show_mouse = false;
			else
				mouse_timer--;
		}
		else
		{
			if (mouse_timer == 30)
				show_mouse = true;
			else
				mouse_timer++;
		}

#pragma region keybord input
		if (GetKey(olc::Key::CTRL).bHeld == false)
		{
			//Alphabet
			for (int i = 1; i <= olc::Key::Z; ++i) {
				olc::Key key = static_cast<olc::Key>(i);
				if (GetKey(key).bPressed) {
					if (GetKey(olc::Key::SHIFT).bHeld)
					{
						char next_char = charKeys[key] - 32;
						add_char(next_char);
					}
					else
					{
						char next_char = charKeys[key];
						add_char(next_char);
					}
					mouse_position.first++;
				}
			}
			//numbers
			for (int i = olc::Key::K0; i <= olc::Key::K9; ++i) {
				olc::Key key = static_cast<olc::Key>(i);
				if (GetKey(key).bPressed) {
					if (GetKey(olc::Key::SHIFT).bHeld)
					{
						if (i == 27)
						{
							add_char('=');
						}
						else if (i == 28)
						{
							add_char('!');
						}
						else if (i == 29)
						{
							add_char('"');
						}
						else if (i == 30)
						{
							add_char('§');
						}
						else if (i == 31)
						{
							add_char('$');
						}
						else if (i == 32)
						{
							add_char('%');
						}
						else if (i == 33)
						{
							add_char('&');
						}
						else if (i == 34)
						{
							add_char('/');
						}
						else if (i == 35)
						{
							add_char('(');

						}
						else if (i == 36)
						{
							add_char(')');
						}
					}
					else
					{
						char next_char = charKeys[key];
						add_char(next_char);
					}
					mouse_position.first++;
				}
			}
			if (GetKey(olc::Key::BACK).bPressed)
			{
				if (mark_all == false)
				{
					if (dialogbox_open == false)
					{
						if (mouse_position.first != 0)
						{
							std::string temp = "";
							for (size_t i = 0; i < strings[mouse_position.second].size() + 1; i++)
							{
								if (i != mouse_position.first - 1)
								{
									temp += strings[mouse_position.second][i];
								}
							}
							strings[mouse_position.second] = temp;
							mouse_position.first--;
						}
						else
						{
							if (mouse_position.second - 1 >= 0)
							{
								mouse_position.first = strings[mouse_position.second - 1].size();
								strings[mouse_position.second - 1] = strings[mouse_position.second - 1] + strings[mouse_position.second];

								//ubdate strings
								std::vector<std::string> temp;
								for (size_t i = 0; i < strings.size(); i++)
								{
									if (i != mouse_position.second)
										temp.push_back(strings[i]);
								}
								strings = temp;
								mouse_position.second--;
							}
						}
					}
					else
					{
						if (dialogbox_txt.size() > 0)
						{
							std::string temp = "";
							for (size_t i = 0; i < dialogbox_txt.size() - 1; i++)
							{
								temp += dialogbox_txt[i];
							}
							dialogbox_txt = temp;
						}
					}
				}
				else
				{
					if (dialogbox_open == false)
					{
						strings.clear();
						mark_all = false;
						mouse_position = { 0,0 };
					}
					else
					{
						dialogbox_txt = "";
					}
				}
			}
			if (GetKey(olc::Key::PERIOD).bPressed)
			{
				if (GetKey(olc::Key::SHIFT).bHeld)
				{
					add_char(':');
				}
				else
				{
					add_char('.');
				}
				mouse_position.first++;
			}
			if (GetKey(olc::Key::ENTER).bPressed)
			{
				if (dialogbox_open == false)
				{
					int max_mouse = ((ScreenHeight() - offset_y) + (line_offset_y * (-1))) / (font_height + line_space_offset);
					if (mouse_position.second >= max_mouse - 2)
					{
						line_offset_y -= font_height + line_space_offset;
						y_changed++;
					}

					if (strings.size() >= mouse_position.second)
					{
						strings.push_back("");
					}
					std::string ubdate_line = "";
					std::string new_line = "";
					for (size_t i = 0; i < strings[mouse_position.second].size(); i++)
					{
						if (i < mouse_position.first)
						{
							ubdate_line += strings[mouse_position.second][i];
						}
						else
						{
							new_line += strings[mouse_position.second][i];
						}
					}
					strings[mouse_position.second] = ubdate_line;
					//strings[mouse_position.second + 1] = new_line;

					std::vector<std::string> temp;
					for (size_t i = 0; i < strings.size(); i++)
					{
						if (i == mouse_position.second + 1)
						{
							temp.push_back(new_line);
						}
						temp.push_back(strings[i]);
					}
					strings = temp;

					//delete empty \0
					std::string temp2 = "";
					for (size_t i = 0; i < strings[mouse_position.second].size(); i++)
					{
						if (strings[mouse_position.second][i] != '\0')
						{
							temp2 += strings[mouse_position.second][i];
						}
					}
					strings[mouse_position.second] = temp2;


					mouse_position.second++;
					line_offset_x = 0;
					mouse_position.first = 0;
				}
				else
				{
					//load file
					std::string line;
					strings.clear();
					std::ifstream myfile(dialogbox_txt);
					if (myfile.is_open())
					{
						while (getline(myfile, line))
						{
							strings.push_back(line);
						}
						myfile.close();
						openfile = dialogbox_txt;
					}
					else
					{
						strings.push_back("Unable to open file");
						openfile = "unnamed";
					}
					dialogbox_open = false;
					offset_y -= dialogbox_size;
				}
			}
			if (GetKey(olc::Key::SUB).bPressed)
			{
				if (GetKey(olc::Key::SHIFT).bHeld)
				{
					add_char('_');
				}
				else
				{
					add_char('-');
				}
				mouse_position.first++;
			}
			if (dialogbox_open == false)
			{
				if (GetKey(olc::Key::TAB).bPressed)
				{
					add_char(' ');
					add_char(' ');
					mouse_position.first += 2;
				}

				if (GetKey(olc::Key::SZ).bPressed)
				{
					if (GetKey(olc::Key::SHIFT).bHeld)
					{
						add_char('?');
					}
					mouse_position.first++;
				}

				if (GetKey(olc::Key::PLUS).bPressed)
				{
					if (GetKey(olc::Key::SHIFT).bHeld)
					{
						add_char('*');
					}
					else
					{
						add_char('+');
					}
					mouse_position.first++;
				}

				if (GetKey(olc::Key::POUND).bPressed)
				{
					if (GetKey(olc::Key::SHIFT).bHeld)
					{
						add_char(39);
					}
					else
					{
						add_char('#');
					}
					mouse_position.first++;
				}

				if (GetKey(olc::Key::LESSTHAN).bPressed)
				{
					if (GetKey(olc::Key::SHIFT).bHeld)
					{
						add_char('>');
					}
					else
					{
						add_char('<');
					}
					mouse_position.first++;
				}

				if (GetKey(olc::Key::COMMA).bPressed)
				{
					if (GetKey(olc::Key::SHIFT).bHeld)
					{
						add_char(';');
					}
					else
					{
						add_char(',');
					}
					mouse_position.first++;
				}






				if (GetKey(olc::Key::SPACE).bPressed)
				{
					add_char(' ');
					mouse_position.first++;
				}

				if (GetKey(olc::Key::DEL).bPressed)
				{
					std::string temp = "";
					for (size_t i = 0; i < strings[mouse_position.second].size() + 1; i++)
					{
						if (i != mouse_position.first)
						{
							temp += strings[mouse_position.second][i];
						}
					}
					strings[mouse_position.second] = temp;
				}

				//Mouse
				if (GetKey(olc::Key::NP_ADD).bPressed)
				{
					increase_font_size();
				}
				if (GetKey(olc::Key::NP_SUB).bPressed)
				{
					decrease_font_size();
				}
				if (GetKey(olc::Key::RIGHT).bPressed)
				{
					if (mouse_position.first + (line_offset_x / font_width) < ((ScreenWidth() - offset_x) / font_width) - 2)
					{
						mouse_position.first++;
					}
					else
					{
						line_offset_x -= font_width;
						mouse_position.first++;
					}
				}
				if (GetKey(olc::Key::LEFT).bPressed)
				{
					if (mouse_position.first > 0)
					{
						if (mouse_position.first + (line_offset_x / font_width) > 0)
						{
							mouse_position.first--;
						}
						else
						{
							line_offset_x += font_width;
							mouse_position.first--;
						}
					}
				}
				if (GetKey(olc::Key::UP).bPressed)
				{
					if (mouse_position.second > 0)
					{
						if (y_changed * font_height != mouse_position.second * font_height)
						{
							mouse_position.second--;
						}
						else
						{
							line_offset_y += font_height + line_space_offset;
							y_changed--;
							mouse_position.second--;
						}
					}
				}
				if (GetKey(olc::Key::DOWN).bPressed)
				{
					int max_mouse = ((ScreenHeight() - offset_y) + (line_offset_y * (-1))) / (font_height + line_space_offset);
					if (mouse_position.second <= max_mouse - 2)
					{
						mouse_position.second++;
					}
					else
					{
						line_offset_y -= font_height + line_space_offset;
						y_changed++;
						mouse_position.second++;
					}
					if (mouse_position.second + 1 >= strings.size())
						strings.push_back("");
				}

				
				//if (GetKey(olc::Key::F5).bPressed)
				//{
				//	myfile.open("auto_generated.cpp");
				//	//delete empty spaces
				//	std::vector<std::string> temp;
				//	for (size_t i = 0; i < strings.size(); i++)
				//	{
				//		std::string new_line = "";
				//		for (size_t p = 0; p < strings[i].size(); p++)
				//		{
				//			if (strings[i][p] != '\0')
				//			{
				//				new_line += strings[i][p];
				//			}
				//		}
				//		temp.push_back(new_line);
				//	}
				//	strings = temp;
				//	for (size_t i = 0; i < strings.size(); i++)
				//	{
				//		myfile << strings[i] << "\n";
				//	}
				//	myfile.close();
				//	std::string command = "test.bat auto_generated.cpp";
				//	std::thread thr(compile, command);

				//	thr.detach();
				//}
			}
		}
		else
		{
			if (GetKey(olc::Key::RIGHT).bPressed)
			{
				if (strings.size() > mouse_position.second)
				{
					format();
					std::vector<std::string> words = split_in_to_words(mouse_position.second);
					int line_length = 0;
					for (size_t i = 0; i < words.size(); i++)
					{
						line_length += words[i].size();
						line_length++;
					}
					line_length--;
					if (line_length > mouse_position.first)
					{

					}
				}
			}

			if (GetKey(olc::Key::A).bPressed)
			{
				//mark markieren
				if (mark_all == true)
				{
					mark_all = false;
				}
				else
				{
					mark_all = true;
				}
			}

			if (GetKey(olc::Key::ALT).bHeld)
			{
				if (GetKey(olc::Key::K7).bPressed)
				{
					add_char('{');
					mouse_position.first++;
				}
				if (GetKey(olc::Key::K0).bPressed)
				{
					add_char('}');
					mouse_position.first++;
				}
				//------
				if (GetKey(olc::Key::SZ).bPressed)
				{
					add_char(92);
					mouse_position.first++;
				}
				//-----
				if (GetKey(olc::Key::LESSTHAN).bPressed)
				{
					add_char('|');
					mouse_position.first++;
				}
			}
			if (GetKey(olc::Key::S).bPressed)
			{
				myfile.open(openfile);
				//delete empty spaces
				format();
				for (size_t i = 0; i < strings.size(); i++)
				{
					myfile << strings[i] << "\n";
				}
				myfile.close();

				save_animation = true;
			}
			if (GetKey(olc::Key::O).bPressed)
			{
				if (dialogbox_open == false)
				{
					dialogbox_open = true;
					offset_y += dialogbox_size;
				}
				else
				{
					dialogbox_open = false;
					offset_y -= dialogbox_size;
				}
			}
			if (GetKey(olc::Key::V).bPressed)
			{
				std::string temp = GetClipboardText();

				if (dialogbox_open == false)
				{
					//zwischenablage in zeilen unterscheiden
					std::vector<std::string> final_str;
					std::string current = "";
					for (size_t i = 0; i < temp.size(); i++)
					{
						if (temp[i] == '\n')
						{
							final_str.push_back(current);
							current = "";
						}
						else
						{
							current += temp[i];
						}
					}
					final_str.push_back(current);

					std::vector<std::string> temp2;
					for (size_t i = 0; i < strings.size(); i++)
					{
						temp2.push_back(strings[i]);
						if (i == mouse_position.second)
						{
							if (final_str.size() > 1)
							{
								for (size_t p = 0; p < final_str.size(); p++)
								{
									temp2.push_back(final_str[p]);
								}
							}
							else //multiline
							{
								if (mouse_position.first < temp2[i].size())
								{
									std::string temp4 = "";
									for (size_t p = 0; p < strings[i].size(); p++)
									{
										if (p == mouse_position.first)
										{
											for (size_t k = 0; k < final_str[0].size(); k++)
											{
												temp4 += final_str[0][k];
												if (mouse_position.first + (line_offset_x / font_width) < (ScreenWidth() / font_width) - 2)
												{
													mouse_position.first++;
												}
												else
												{
													line_offset_x -= font_width;
													mouse_position.first++;
												}
											}
										}
										temp4 += strings[i][p];
									}
									temp2[i] = temp4;
								}
								else //single line
								{
									std::string temp4 = strings[i];
									for (size_t p = 0; p < mouse_position.first - strings[i].size(); p++)
									{
										temp4 += " ";
										if (mouse_position.first + (line_offset_x / font_width) < (ScreenWidth() / font_width) - 2)
										{
											mouse_position.first++;
										}
										else
										{
											line_offset_x -= font_width;
											mouse_position.first++;
										}
									}
									for (size_t p = 0; p < final_str[0].size(); p++)
									{
										temp4 += final_str[0][p];
										if (mouse_position.first + (line_offset_x / font_width) < (ScreenWidth() / font_width) - 2)
										{
											mouse_position.first++;
										}
										else
										{
											line_offset_x -= font_width;
											mouse_position.first++;
										}
									}
									//temp4 += final_str[0];
									temp2[i] = temp4;
								}
							}
						}
					}
					strings = temp2;
				}
				else
				{
					dialogbox_txt += temp;
				}
			}

		}

		if (GetMouseWheel() == -120)
		{
			int max_mouse = ((ScreenHeight() - offset_y) + (line_offset_y * (-1))) / (font_height + line_space_offset);
			if (mouse_position.second <= max_mouse - 2)
			{
				mouse_position.second++;
			}
			else
			{
				line_offset_y -= font_height + line_space_offset;
				y_changed++;
				mouse_position.second++;
			}
			if (mouse_position.second + 1 >= strings.size())
				strings.push_back("");
		}
		else if (GetMouseWheel() == 120)
		{
			if (mouse_position.second > 0)
			{
				if (y_changed * font_height != mouse_position.second * font_height)
				{
					mouse_position.second--;
				}
				else
				{
					line_offset_y += font_height + line_space_offset;
					y_changed--;
					mouse_position.second--;
				}
			}
		}
		else if (GetMouseWheel() == -240)
		{
			int max_mouse = ((ScreenHeight() - offset_y) + (line_offset_y * (-1))) / (font_height + line_space_offset);
			if (mouse_position.second <= max_mouse - 2)
			{
				mouse_position.second++;
			}
			else
			{
				line_offset_y -= font_height + line_space_offset;
				y_changed++;
				mouse_position.second++;
			}
			if (mouse_position.second + 1 >= strings.size())
				strings.push_back("");
		}
		else if (GetMouseWheel() == 240)
		{
			if (mouse_position.second > 0)
			{
				if (y_changed * font_height != mouse_position.second * font_height)
				{
					mouse_position.second--;
				}
				else
				{
					line_offset_y += font_height + line_space_offset;
					y_changed--;
					mouse_position.second--;
				}
			}
		}

#pragma endregion
		if (dialogbox_open == true)
		{
			//dialogbox open

			//draw window
			for (size_t i = 0; i < 4; i++)
			{
				DrawRect(i, info_size + i, (ScreenWidth() - 1) - (i * 2), dialogbox_size - (i * 2), olc::Pixel(70, 130, 180));
			}
			DrawString(20, info_size + 20, "What file do you want to open?", olc::Pixel(240, 230, 140), 2);
			//FillRect((offset_x + line_offset_x) + (font_width * mouse_position.first), offset_y + (mouse_position.second * (font_height + line_space_offset) + line_offset_y) + font_height, font_width, font_height / 4, olc::WHITE);
			//FillRect(20, (info_size + 20) + ((line_space_offset + font_height) * 2), 16, 14 / 4, olc::WHITE);
			DrawString(20, (info_size + 20) + (line_space_offset + font_height), "->", olc::RED, 2);
			DrawString(20 + (16 * 2), (info_size + 20) + (line_space_offset + font_height), dialogbox_txt, olc::Pixel(224, 238, 224), 2);
		}



		//draw words / lines
		int max_lines = ScreenHeight() / (line_space_offset + font_height);
		std::vector<int> lines_to_show;
		if (max_lines > strings.size())
		{
			for (size_t i = 0; i < strings.size(); i++)
			{
				lines_to_show.push_back(i);
			}
		}
		else
		{
			for (size_t i = 0; i < max_lines; i++)
			{
				lines_to_show.push_back(y_changed + i);
			}
		}


		int height = 0;
		for (size_t p = 0; p < lines_to_show.size(); p++)
		{
			std::vector<std::string> test = split_in_to_words(lines_to_show[p]);
			int length = 0;
			for (size_t i = 0; i < test.size(); i++)
			{
				color has_color = grey;
				for (const auto &word : blue_words)
				{
					if (test[i] == word)
					{
						has_color = blue;
					}
				}
				for (const auto &word : orange_words)
				{
					if (test[i] == word)
					{
						has_color = orange;
					}
				}
				for (const auto &word : honeydew_words)
				{
					if (test[i] == word)
					{
						has_color = honeydew;
					}
				}

				//words
				if (has_color == blue)
				{
					if (mark_all == false)
						DrawString((line_offset_x + offset_x) + length, (offset_y)+height, test[i], olc::Pixel(100, 149, 237), font_size);
					else
						DrawString((line_offset_x + offset_x) + length, (offset_y)+height, test[i], olc::Pixel(255 - 100, 255 - 149, 255 - 237), font_size);
				}
				else if (has_color == orange)
				{
					if (mark_all == false)
						DrawString((line_offset_x + offset_x) + length, (offset_y)+height, test[i], olc::Pixel(218, 165, 32), font_size);
					else
						DrawString((line_offset_x + offset_x) + length, (offset_y)+height, test[i], olc::Pixel(255 - 218, 255 - 165, 255 - 32), font_size);
				}
				else if (has_color == honeydew)
				{
					if (mark_all == false)
						DrawString((line_offset_x + offset_x) + length, (offset_y)+height, test[i], olc::Pixel(0, 206, 209), font_size);
					else
						DrawString((line_offset_x + offset_x) + length, (offset_y)+height, test[i], olc::Pixel(255 - 0, 255 - 206, 255 - 209), font_size);
				}
				else
				{
					if (mark_all == false)
						DrawString((line_offset_x + offset_x) + length, (offset_y)+height, test[i], olc::Pixel(224, 238, 224), font_size);
					else
						DrawString((line_offset_x + offset_x) + length, (offset_y)+height, test[i], olc::Pixel(255 - 224, 255 - 238, 255 - 224), font_size);
				}

				//symbols
				for (size_t m = 0; m < test[i].size(); m++)
				{
					for (const auto &symbol : dark_orange_symbols)
					{
						if (test[i][m] == symbol)
						{
							if (mark_all == false)
								DrawString(((line_offset_x + offset_x) + length) + (m * font_width), (offset_y)+height, std::string(&symbol, 1), olc::Pixel(255, 98, 0), font_size);
							else
								DrawString(((line_offset_x + offset_x) + length) + (m * font_width), (offset_y)+height, std::string(&symbol, 1), olc::Pixel(255 - 255, 255 - 98, 255 - 0), font_size);
						}
					}
					for (const auto &symbol : fuchsia_words)
					{
						if (test[i][m] == symbol)
						{
							if (mark_all == false)
								DrawString(((line_offset_x + offset_x) + length) + (m * font_width), (offset_y)+height, std::string(&symbol, 1), olc::Pixel(255, 0, 255), font_size);
							else
								DrawString(((line_offset_x + offset_x) + length) + (m * font_width), (offset_y)+height, std::string(&symbol, 1), olc::Pixel(255 - 255, 255 - 0, 255 - 255), font_size);
						}
					}
					if (test[i][m] == '"')
					{
						if (mark_all == false)
							DrawString(((line_offset_x + offset_x) + length) + (m * font_width), (offset_y)+height, "\"", olc::Pixel(170, 255, 0), font_size);
						else
							DrawString(((line_offset_x + offset_x) + length) + (m * font_width), (offset_y)+height, "\"", olc::Pixel(255 - 170, 255 - 255, 255 - 0), font_size);
					}
					if (test[i][m] == '/')
					{
						if (mark_all == false)
							DrawString(((line_offset_x + offset_x) + length) + (m * font_width), (offset_y)+height, "/", olc::Pixel(0, 255, 0), font_size);
						else
							DrawString(((line_offset_x + offset_x) + length) + (m * font_width), (offset_y)+height, "/", olc::Pixel(255 - 0, 255 - 255, 255 - 0), font_size);
					}
				}
				length += (test[i].size()*font_width) + font_width;
			}
			height += font_height + line_space_offset;
		}

		//draw row_numbers nummern an der seite malen
		for (size_t i = 0; i < max_lines; i++)
		{
			if (i < strings.size())
			{
				DrawString(10 + line_offset_x, ((line_space_offset + font_height) * i) + offset_y, std::to_string(i + y_changed), olc::DARK_GREEN, font_size);
			}
		}

		//set x_offset so that the text moves to make space for the numbers
		std::string anzahl = std::to_string(strings.size());
		offset_x = (anzahl.size() * font_width) + 20;

		//show time, info etc...

		FillRect(0, 0, ScreenWidth(), info_size, olc::GREY);

		time_t result = time(NULL);

		// ctime() used to give the present time 
		char str[26];
		ctime_s(str, sizeof str, &result);
		std::string current_time = "";
		int counter = 0;
		for (const auto &it : str)
		{
			counter++;
			if (counter > 11 and counter < 20)
				current_time += it;
		}
		//time
		DrawString((ScreenWidth() / 2) - (16 * 4), info_size / 2 - 7, current_time, olc::BLACK, 2);


		std::string row_info = "row " + std::to_string(mouse_position.second) + " / " + std::to_string(strings.size());
		DrawString(ScreenWidth() / 2 + (16 * 8), info_size / 2 - 7, row_info, olc::Pixel(112, 128, 240), 2);

		//show opend file name
		std::string filename = "";
		for (size_t i = 0; i < openfile.size(); i++)
		{
			if (openfile[i] == '/' or openfile[i] == '\\')
			{
				filename = "";
			}
			else
			{
				filename += openfile[i];
			}
		}
		DrawString(50, info_size / 2 - 7, "-" + filename + "-", olc::Pixel(139, 71, 38), 2);
#pragma region draw_little_pacman_animation
		//pacman start ist direkt rechts von der uhr mit einem offset von der länge von der größe von pacman
		int pacman_start = ((ScreenWidth() / 2) + ((8 * 16) / 2) + pacman_sprite.width) + (row_info.size() * 16) + (4 * 16);
		if (pacman_timer == 60)
		{
			if (pacman_pos > ((ScreenWidth() - pacman_start) / pacman_sprite.width) - 2)
				pacman_pos = 0;
			else
				pacman_pos++;

			pacman_timer = 0;
		}
		else
			pacman_timer++;

		//draw food
		for (size_t i = 0; i < (ScreenWidth() - pacman_start) / pacman_sprite.width; i++)
		{
			if (i > pacman_pos)
			{
				FillCircle(pacman_start + (i * pacman_sprite.width) + 10, (info_size / 2) - (pacman_sprite.height / 2) + 10, 5, olc::WHITE);
			}
		}

		SetPixelMode(olc::Pixel::MASK);
		DrawSprite(pacman_start + (pacman_sprite.width * pacman_pos), (info_size / 2) - (pacman_sprite.height / 2), &pacman_sprite, 1);
#pragma endregion
#pragma region save_animation
		if (save_animation == true)
		{
			//saved... (8)-------
			//draw window
			if (save_animation_timer == 1)
			{
				if (save_animation_step != 0)
				{
					save_animation_step--;
				}
				else
				{
					if (save_animation_wait > 0)
					{
						save_animation_wait--;
					}
					else
					{
						save_animation_wait = 60;
						save_animation = false;
						save_animation_step = 30;
					}

				}
				save_animation_timer = 0;
			}
			else
			{
				save_animation_timer++;
			}

			/*for (size_t i = 0; i < 4; i++)
			{
				DrawRect(i, info_size + i, (ScreenWidth() - 1) - (i * 2), dialogbox_size - (i * 2), olc::Pixel(70, 130, 180));
			}*/


			//FillRect((((ScreenWidth() / 4) * 3) - (16 * 8)), ((ScreenHeight() - 28) + save_animation_step), 16 * 9, 40, olc::Pixel(255, 140, 0));
			FillRect((((ScreenWidth() / 4) * 3) - (16 * 8)), ((ScreenHeight() - 28) + save_animation_step), 16 * 9, 40, olc::BLACK);
			for (size_t i = 0; i < 3; i++)
			{
				DrawRect((((ScreenWidth() / 4) * 3) - (16 * 8)) + i, ((ScreenHeight() - 28) + save_animation_step) + i, (16 * 9) - (i * 2), 40 - (i * 2), olc::Pixel(70, 130, 180));
			}
			//DrawRect(((ScreenWidth() / 4) * 3) - (16 * 8), ScreenHeight() - 28, 16 * 9, 28, olc::Pixel(70, 130, 180));
			DrawString(((ScreenWidth() / 4) * 3) - (16 * 8) + 8, (ScreenHeight() - 21) + save_animation_step, "saved...", olc::WHITE, 2);

		}
#pragma endregion

		return true;
	}
};


int main()
{
	Editor Window;
	//1200,700
	if (Window.Construct(1200, 700, 1, 1, false, true))
		Window.Start();

	return 0;
}
