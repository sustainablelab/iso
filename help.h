#ifndef __HELP_H__
#define __HELP_H__

const char * main_overlay_text = ""
    "- Press `:` (COLON) to navigate this debug overlay.\n"
    "  A green highlight appears.\n\n"
    "  - Press `j,k,h,l` to move this highlight.\n\n"
    "- Press `i` (insert) to edit the highlighted value (RGBA only).\n"
    "  - Press `Esc` to leave insert mode.\n"
    "  - Very limited text editing ability:\n"
    "    - enter numbers (value is 0 to 255)\n"
    "    - Backspace to erase\n"
    "    - Enter to submit value\n"
    "    - Shift+Enter to submit value and move onto next input\n"
    "    - Press `Esc` to leave insert mode.\n\n"
    "- Press `Esc` to leave the highlight mode.\n"
    "- Press `Esc` again and the game quits.";


#endif // __HELP_H__
