#ifndef __HELP_H__
#define __HELP_H__

const char * hint_text = "(Press ? for help, Tab toggles this HUD)";
const char * help_text = ""
    "- Press `:` (COLON) to navigate this debug overlay.\n"
    "  A green highlight appears.\n"
    "\n"
    "  - Press `j,k,h,l` to move this highlight.\n"
    "  - Press `Up/Down` to continuous inc/dec highlighted value.\n"
    "  - Press `Shift Up/Down` to step inc/dec highlighted value.\n"
    "  - NOTE: values are clamped during inc/dec\n"
    "    - inc/dec RGBA clamp: [0,255]\n"
    "    - inc/dec X    clamp: [0,win width]\n"
    "    - inc/dec Y    clamp: [0,win height]\n"
    "\n"
    "- Press `i` (insert) to edit the highlighted value (RGBA only).\n"
    "  - Press `Esc` to leave insert mode.\n"
    "  - Very limited text editing ability:\n"
    "    - enter whole numbers (anything after a non-digit is ignored)\n"
    "    - Backspace to erase\n"
    "    - Enter to submit value\n"
    "    - Shift+Enter to submit value and move onto next input\n"
    "    - Press `Esc` to leave insert mode.\n"
    "\n"
    "- Press `Esc` to leave the highlight mode.\n"
    "- Press `Esc` again and the game quits.";


#endif // __HELP_H__
