struct Ui {
    name: Text,
    elements: List<UiElement>,
    on_draw: Option<fn()>,
    selected: int,
}
impl Ui {
    fn new(name: string, elements: List<UiElement>) -> Ui {
        Ui { name: Text { text: name }, elements: elements, on_draw: Option::None, selected: 0 }
    }
    fn new_with_selected(name: string, selected: int, elements: List<UiElement>) -> Ui {
        Ui { name: Text { text: name }, elements: elements, on_draw: Option::None, selected: selected }
    }

    fn new_filechooser(name: string, file_list: List<string>, onclick: fn(string)) -> Ui {
        let mut files = List::of(
            UiElement::Button(UiButton {
                label: Text { text: "Back" },
                onclick: fn(label: Text) { leave_ui() },
            }),
            UiElement::Input(Input {
                label: Text { text: "File name" },
                input: "",
                onclick: fn(input: string) {
                    if input.len_utf8() == 0 {
                        return;
                    }
                    onclick(input);
                },
                onchange: fn(input: string) {}
            }),
        );
        for file in file_list {
            files.push(UiElement::Button(UiButton {
                label: Text { text: file },
                onclick: fn(label: Text) {
                    onclick(label.text);
                },
            }));
        }
        Ui::new(name, files)
    }
}

enum UiElement {
    Button(UiButton),
    Input(Input),
    FloatInput(FloatInput),
    Slider(Slider),
    Chooser(Chooser),
}
struct UiButton {
    label: Text,
    onclick: fn(Text),
}
struct Input {
    label: Text,
    input: string,
    onclick: fn(string),
    onchange: fn(string),
}
struct FloatInput {
    label: Text,
    input: string,
    onclick: fn(string),
    onchange: fn(string),
}
struct Text {
    text: string,
}
struct Slider {
    label: Text,
    content: Text,
    onleft: fn(),
    onright: fn(),
}
struct Chooser {
    label: Text,
    selected: int,
    options: List<Text>,
    onchange: fn(int),
}

static mut UI_STACK: List<Ui> = List::new();
static mut LSHIFT_PRESSED = false;
static mut RSHIFT_PRESSED = false;
static mut LCTRL_PRESSED = false;
static mut RCTRL_PRESSED = false;

fn enter_ui(ui: Ui) {
    UI_STACK.push(ui);
}
fn leave_ui() {
    if UI_STACK.len() > 1 {
        UI_STACK.pop();
    }
}

fn on_key_down(key_code: int, character_code: int, is_repeat: bool) {
    let key = KeyCode::from_large(key_code);
    if key.to_small() == KEY_LEFT_SHIFT.to_small() {
        LSHIFT_PRESSED = true;
    } else if key.to_small() == KEY_RIGHT_SHIFT.to_small() {
        RSHIFT_PRESSED = true;
    } else if key.to_small() == KEY_LEFT_CTRL.to_small() {
        LCTRL_PRESSED = true;
    } else if key.to_small() == KEY_RIGHT_CTRL.to_small() {
        RCTRL_PRESSED = true;
    }
    if key.to_small() == KEY_M.to_small() && !TAS_STATE.step_frame_mode && UI_STACK.len() == 1 {
        enter_ui(create_base_menu());
    }
    if key.to_small() == KEY_N.to_small() {
        Tas::set_input_mode_ui_only();
        Tas::flush_pressed_keys();
    }
    if key.to_small() == KEY_P.to_small() {
        Tas::get_location_and_log();
    }
    if key.to_small() == KEY_S.to_small() {
        Tas::set_input_mode_game_only();
    }

    //Ledge, jump_pad, WallJump, Swim, Lifts, Pipes

    // if key.to_small() == KEY_1.to_small(){
    //     ARCHIPELAGO_STATE.ledge_grab = !ARCHIPELAGO_STATE.ledge_grab;
    //     Tas::abilities_set_ledge_grab(ARCHIPELAGO_STATE.ledge_grab);
    //     log(f"Ledge Grab set to {ARCHIPELAGO_STATE.ledge_grab}");
    // }
    // if key.to_small() == KEY_2.to_small(){
    //     ARCHIPELAGO_STATE.jump_pads = !ARCHIPELAGO_STATE.jump_pads;
    //     Tas::abilities_set_jump_pads(ARCHIPELAGO_STATE.jump_pads);
    //     log(f"Jump Pad set to {ARCHIPELAGO_STATE.jump_pads}");
    // }
    // if key.to_small() == KEY_3.to_small(){
    //     ARCHIPELAGO_STATE.wall_jump = (ARCHIPELAGO_STATE.wall_jump + 1) % 3;
    //     Tas::abilities_set_wall_jump(ARCHIPELAGO_STATE.wall_jump, true);
    //     log(f"Wall Jump set to {ARCHIPELAGO_STATE.wall_jump}");
    // }
    // if key.to_small() == KEY_4.to_small(){
    //     ARCHIPELAGO_STATE.swim = !ARCHIPELAGO_STATE.swim;
    //     Tas::abilities_set_swim(ARCHIPELAGO_STATE.swim);
    //     log(f"Swim set to {ARCHIPELAGO_STATE.swim}");
    // }
    // if key.to_small() == KEY_5.to_small(){
    //     ARCHIPELAGO_STATE.lifts = !ARCHIPELAGO_STATE.lifts;
    //     Tas::abilities_set_lifts(ARCHIPELAGO_STATE.lifts);
    //     log(f"Lifts set to {ARCHIPELAGO_STATE.lifts}");
    // }
    // if key.to_small() == KEY_6.to_small(){
    //     ARCHIPELAGO_STATE.pipes = !ARCHIPELAGO_STATE.pipes;
    //     Tas::abilities_set_pipes(ARCHIPELAGO_STATE.pipes);
    //     log(f"Pipes set to {ARCHIPELAGO_STATE.pipes}");
    // }
    // if key.to_small() == KEY_7.to_small(){
    //     let current_loc = Tas::get_location();
    //     let loc = Location { x: current_loc.x, y: current_loc.y, z: current_loc.z + 10000. };
    //     Tas::set_location(loc);
    //     log(f"Teleported up by 10000 units");
    // }

    match UI_STACK.last() {
        Option::Some(ui) => ui.onkey(key),
        Option::None => (),
    }
    for comp in CURRENT_COMPONENTS {
        let on_key_down_always = comp.on_key_down_always;
        on_key_down_always(key, is_repeat);
        // don't trigger key events while in the menu
        if UI_STACK.len() == 1 {
            let on_key_down = comp.on_key_down;
            on_key_down(key, is_repeat);
        }
    }
}
fn on_key_up(key_code: int, character_code: int, is_repeat: bool) {
    let key = KeyCode::from_large(key_code);
    if key.to_small() == KEY_LEFT_SHIFT.to_small() {
        LSHIFT_PRESSED = false;
    } else if key.to_small() == KEY_RIGHT_SHIFT.to_small() {
        RSHIFT_PRESSED = false;
    } else if key.to_small() == KEY_LEFT_CTRL.to_small() {
        LCTRL_PRESSED = false;
    } else if key.to_small() == KEY_RIGHT_CTRL.to_small() {
        RCTRL_PRESSED = false;
    }
    for comp in CURRENT_COMPONENTS {
        let on_key_up_always = comp.on_key_up_always;
        on_key_up_always(key);
        // don't trigger key events while in the menu
        if UI_STACK.len() == 1 {
            let on_key_up = comp.on_key_up;
            on_key_up(key);
        }
    }
}

fn on_key_char(character: string, is_repeat: bool) {
    match UI_STACK.last() {
        Option::Some(ui) => ui.onchar(character),
        Option::None => (),
    }

    for comp in CURRENT_COMPONENTS {
        let on_key_char_always = comp.on_key_char_always;
        on_key_char_always(character);

        // don't trigger key events while in the menu
        if UI_STACK.len() == 1 {
            let on_key_char = comp.on_key_char;
            on_key_char(character);
        }
    }
}

fn on_mouse_move(x: int, y: int) {
    for comp in CURRENT_COMPONENTS {
        let on_mouse_move = comp.on_mouse_move;
        on_mouse_move(x, y);
    }
}
fn draw_hud() {
    for component in CURRENT_COMPONENTS {
        let draw_hud_always = component.draw_hud_always;
        draw_hud_always();
    }
    match UI_STACK.last() {
        Option::Some(ui) => ui.draw(),
        Option::None => (),
    }
    draw_log_messages();
}


fn on_resolution_change() {
    for component in CURRENT_COMPONENTS {
        let on_resolution_change = component.on_resolution_change;
        on_resolution_change();
    }
}

fn on_menu_open() {
    for component in CURRENT_COMPONENTS {
        let on_menu_open = component.on_menu_open;
        on_menu_open();
    }
    leave_ui();
    leave_ui();
    leave_ui();
}

static COLOR_BLACK = Color { red: 0., green: 0., blue: 0., alpha: 1. };
static COLOR_RED = Color { red: 1., green: 0., blue: 0., alpha: 1. };
static COLOR_GREEN = Color { red: 0., green: 1., blue: 0., alpha: 1. };
static COLOR_YELLOW = Color { red: 1., green: 1., blue: 0., alpha: 1. };
static COLOR_WHITE = Color { red: 1., green: 1., blue: 1., alpha: 1. };
static COLOR_BLUE = Color { red: 0., green: 0., blue: 1., alpha: 1. };

impl Ui {
    fn onclick(self) {
        match self.elements.get(self.selected) {
            Option::Some(element) => element.onclick(),
            Option::None => (),
        }
    }
    fn onkey(mut self, key: KeyCode) {
        if key.to_small() == KEY_RETURN.to_small() {
            self.onclick();
        } else if key.to_small() == KEY_DOWN.to_small() {
            self.selected = if self.selected == self.elements.len()-1 {
                0
            } else {
                self.selected + 1
            };
        } else if key.to_small() == KEY_UP.to_small() {
            self.selected = if self.selected == 0 {
                self.elements.len() - 1
            } else {
                self.selected - 1
            };
        }
        match self.elements.get(self.selected) {
            Option::Some(element) => element.onkey(key),
            Option::None => (),
        }
    }

    fn onchar(self, character: string) {
        match self.elements.get(self.selected) {
            Option::Some(element) => element.onchar(character),
            Option::None => (),
        }
    }

    fn draw(self) {
        // This padding dictates how much space there will be between elements. Got no clue why it's done like this.
        let padding = 48.;
        match self.on_draw {
            Option::Some(f) => f(),
            Option::None => (),
        }
        Tas::draw_text(DrawText {
            text: self.name.text,
            color: COLOR_BLACK,
            x: 8.,
            y: 8.,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        });
        Tas::draw_text(DrawText {
            text: self.name.text,
            color: COLOR_BLACK,
            x: 12.,
            y: 12.,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        });
        Tas::draw_text(DrawText {
            text: self.name.text,
            color: COLOR_BLACK,
            x: 8.,
            y: 12.,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        });
        Tas::draw_text(DrawText {
            text: self.name.text,
            color: COLOR_BLACK,
            x: 12.,
            y: 8.,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        });
        Tas::draw_text(DrawText {
            text: self.name.text,
            color: COLOR_WHITE,
            x: 10.,
            y: 10.,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        });

        let mut i = 0;
        for element in self.elements {
            let color = if self.selected == i { COLOR_BLUE } else { COLOR_BLACK };
            element.draw(padding + i.to_float() * padding, color);
            i = i + 1;
        }
    }
}

impl UiElement {
    fn onclick(self) {
        match self {
            UiElement::Button(button) => button.onclick(),
            UiElement::Input(input) => input.onclick(),
            UiElement::FloatInput(input) => input.onclick(),
            UiElement::Slider(slider) => (),
            UiElement::Chooser(chooser) => (),
        }
    }
    fn onkey(self, key: KeyCode) {
        match self {
            UiElement::Button(button) => (),
            UiElement::Input(input) => input.onkey(key),
            UiElement::FloatInput(input) => input.onkey(key),
            UiElement::Slider(slider) => slider.onkey(key),
            UiElement::Chooser(chooser) => chooser.onkey(key),
        }
    }
    fn onchar(self, c: string) {
        match self {
            UiElement::Button(button) => (),
            UiElement::Input(input) => input.onchar(c),
            UiElement::FloatInput(input) => input.onchar(c),
            UiElement::Slider(slider) => (),
            UiElement::Chooser(chooser) => (),
        }
    }
    fn draw(self, y: float, color: Color) {
        match self {
            UiElement::Button(button) => button.draw(y, color),
            UiElement::Input(input) => input.draw(y, color),
            UiElement::FloatInput(input) => input.draw(y, color),
            UiElement::Slider(slider) => slider.draw(y, color),
            UiElement::Chooser(chooser) => chooser.draw(y, color),
        }
    }
}

impl UiButton {
    fn onclick(self) {
        let f = self.onclick;
        f(self.label);
    }
    fn draw(self, y: float, color: Color) {
        Tas::draw_text(DrawText {
            text: f"    {self.label.text}",
            color: color,
            x: 0.,
            y: y,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        })
    }
}
impl Input {
    fn onclick(self) {
        let f = self.onclick;
        f(self.input);
    }
    fn onkey(mut self, key: KeyCode) {
        if key.to_small() == KEY_BACKSPACE.to_small() {
            self.input = self.input.slice(0, -1);
        } else if key.to_small() == KEY_V.to_small() && (LCTRL_PRESSED || RCTRL_PRESSED) {
            self.input = f"{self.input}{Tas::get_clipboard()}";
        } else if key.to_small() == KEY_D.to_small() && (LCTRL_PRESSED || RCTRL_PRESSED) {
            self.input = "";
        }
        let onchange = self.onchange;
        onchange(self.input);
    }
    fn onchar(mut self, c: string) {
        self.input = f"{self.input}{c}";
        let onchange = self.onchange;
        onchange(self.input);
    }
    fn draw(self, y: float, color: Color) {
        Tas::draw_text(DrawText {
            text: f"    {self.label.text}: {self.input}",
            color: color,
            x: 0.,
            y: y,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        })
    }
}

static FLOAT_CHARS = List::of("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "+", "-", ".");

impl FloatInput {
    fn onclick(self) {
        let f = self.onclick;
        f(self.input);
    }
    fn onkey(mut self, key: KeyCode) {
        if key.to_small() == KEY_BACKSPACE.to_small() {
            self.input = self.input.slice(0, -1);
        } else if key.to_small() == KEY_V.to_small() && (LCTRL_PRESSED || RCTRL_PRESSED) {
            self.input = f"{self.input}{Tas::get_clipboard()}";
        } else if key.to_small() == KEY_D.to_small() && (LCTRL_PRESSED || RCTRL_PRESSED) {
            self.input = "";
        }

        let onchange = self.onchange;
        onchange(self.input);
    }
    fn onchar(mut self, c: string) {
        if FLOAT_CHARS.contains(c) {
            self.input = f"{self.input}{c}";
        }

        let onchange = self.onchange;
        onchange(self.input);
    }
    fn draw(self, y: float, color: Color) {
        Tas::draw_text(DrawText {
            text: f"    {self.label.text}: {self.input}",
            color: color,
            x: 0.,
            y: y,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        })
    }
}

impl Slider {
    fn onkey(self, key: KeyCode) {
        if key.to_small() == KEY_LEFT.to_small() {
            let f = self.onleft;
            f();
        } else if key.to_small() == KEY_RIGHT.to_small() {
            let f = self.onright;
            f();
        }
    }
    fn draw(self, y: float, color: Color) {
        Tas::draw_text(DrawText {
            text: f"    {self.label.text}: < {self.content.text} >",
            color: color,
            x: 0.,
            y: y,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        })
    }
}

impl Chooser {
    fn onkey(mut self, key: KeyCode) {
        if key.to_small() == KEY_RIGHT.to_small() {
            self.selected = if self.selected  == self.options.len()-1 {
                0
            } else {
                self.selected + 1
            };
            let f = self.onchange;
            f(self.selected);
        } else if key.to_small() == KEY_LEFT.to_small() {
            self.selected = if self.selected == 0 {
                self.options.len() - 1
            } else {
                self.selected - 1
            };
            let f = self.onchange;
            f(self.selected);
        }
    }
    fn draw(self, y: float, color: Color) {
        Tas::draw_text(DrawText {
            text: f"    {self.label.text}: < {self.options.get(self.selected).unwrap().text} >",
            color: color,
            x: 0.,
            y: y,
            scale: SETTINGS.ui_scale,
            scale_position: true,
        })
    }
}

