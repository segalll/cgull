#include "buffer.h"

#include "utf8.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace cgull {
	buffer::buffer() {
		content = {};

		from.stream.open("A.java");

		std::string line;
		while (std::getline(from.stream, line)) {
		    content.push_back(utf8::utf8to32(line));
		}

		from.stream.close();
	}

	void buffer::enter_char(char32_t new_char) {
		content.back() += new_char;
		cursor.col += 1;
	}

	void buffer::new_line() {
		content.push_back(line());
		cursor.row += 1;
		cursor.col = 0;
	}

	void buffer::tab() {
		content[cursor.row] += U"    "; // 4 spaces
		cursor.col += 4;
	}

	void buffer::backspace() {
		if (cursor.col > 0) {
			cursor.col -= 1;
			content[cursor.row].erase(content[cursor.row].begin() + cursor.col);
		} else if (cursor.row > 0) {
			cursor.col = content[cursor.row - 1].size();
			content[cursor.row - 1] += content[cursor.row];
			content.erase(content.begin() + cursor.row);
			cursor.row -= 1;
		}
	}

	void buffer::cursor_left() {
		if (cursor.col == 0) {
			if (cursor.row != 0) {
				cursor.row = cursor.row - 1 > 0 ? cursor.row - 1 : 0;
				cursor.col = content[cursor.row].size();
			}
		} else {
			cursor.col -= 1;
		}
	}

	void buffer::cursor_right() {
		if (cursor.col >= content[cursor.row].size()) {
			if (cursor.row < content.size() - 1) {
				cursor.row += 1;
				cursor.col = 0;
			}
		} else {
			cursor.col += 1;
		}
	}

	void buffer::cursor_up() {
		if (cursor.row == 0) {
			cursor.col = 0;
		} else {
			cursor.row -= 1;
			cursor.col = content[cursor.row].size() < cursor.col ? content[cursor.row].size() : cursor.col;
		}
	}

	void buffer::cursor_down() {
		if (cursor.row >= content.size() - 1) {
			cursor.col = content.back().size();
		} else {
			cursor.row += 1;
			cursor.col = content[cursor.row].size() < cursor.col ? content[cursor.row].size() : cursor.col;
		}
	}

	void buffer::save() {
		from.stream.open("A.java");
		std::string out;
		for (const line& l : content) {
			out += utf8::utf32to8(l) + "\n";
		}
		from.stream << out;

		from.stream.close();
	}
}