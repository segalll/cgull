#include "buffer.h"

namespace cgull {
	buffer::buffer() {
		content = {{}};
	}

	void buffer::backspace() {
		if (content.back().size() > 0) {
			content.back().pop_back();
			cursor.col -= 1;
		} else if (content.size() > 1) {
			content.pop_back();
			cursor.row -= 1;
			cursor.col = content.back().size();
		}
	}

	void buffer::enter_char(unsigned int new_char) {
		content.back().push_back(new_char);
		cursor.col += 1;
	}

	void buffer::new_line() {
		content.push_back(line());
		cursor.row += 1;
		cursor.col = 0;
	}
}