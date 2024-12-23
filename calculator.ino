// C++ code
#include <LiquidCrystal.h>
#include <stdint.h>

#define ANALOG_NUMPAD_PIN_SET_1 A2
#define ANALOG_NUMPAD_PIN_SET_2 A3

#define ANALOG_CALCULATE_FINAL_PIN A1
#define ANALOG_OPERATORS_PIN A0

#define DIG2CHAR(X) (X + '0')

// Liquid Crystal horizontal length is 16 blocks
#define MAX_EXPRESSION_LENGTH 16

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

typedef enum EnumAnalogButtonCode: int32_t { // c++ explicit type definition
  // default values for numpad sets
  ANALOG_NUMPAD_SET_1_DEFAULT_VAL=0,
  ANALOG_NUMPAD_SET_2_DEFAULT_VAL=0,
  ANALOG_CALCULATE_DEFAULT_VAL=1023,
  ANALOG_OPERATORS_DEFAULT_VAL=1023,

  // analog numpad button codes
  ANALOG_CODE_BUTTON_0=1023, ANALOG_CODE_BUTTON_1=465, ANALOG_CODE_BUTTON_2=455,
  ANALOG_CODE_BUTTON_3=292, ANALOG_CODE_BUTTON_4=279, ANALOG_CODE_BUTTON_5=205,
  ANALOG_CODE_BUTTON_6=186, ANALOG_CODE_BUTTON_7=146, ANALOG_CODE_BUTTON_8=114,
  ANALOG_CODE_BUTTON_9=93,

  // analog operator button codes
  ANALOG_CODE_BUTTON_ADDITION=568, ANALOG_CODE_BUTTON_SUBTRACTION=909,
  ANALOG_CODE_BUTTON_MULTIPLICATION=744, ANALOG_CODE_BUTTON_DIVISION=837,

  // analog other button codes
  ANALOG_CODE_BUTTON_CALCULATE=0,

  // special
  ANALOG_NO_BUTTON_PRESSED=-1
} analog_button_code_t;

typedef enum EnumAnalogNumpadButtonLabels: uint32_t { // c++ notation
  ANALOG_LABEL_BUTTON_0, ANALOG_LABEL_BUTTON_1, ANALOG_LABEL_BUTTON_2,
  ANALOG_LABEL_BUTTON_3, ANALOG_LABEL_BUTTON_4, ANALOG_LABEL_BUTTON_5,
  ANALOG_LABEL_BUTTON_6, ANALOG_LABEL_BUTTON_7, ANALOG_LABEL_BUTTON_8,
  ANALOG_LABEL_BUTTON_9, ANALOG_LABEL_BUTTON_UNKNOWN
} analog_numpad_button_label_t;

typedef enum EnumAnalogOperatorButtonLabels: unsigned char { // c++ notation
  ANALOG_LABEL_OPERATOR_ADDITION='+', ANALOG_LABEL_OPERATOR_SUBTRACTION='-',
  ANALOG_LABEL_OPERATOR_MULTIPLICATION='*', ANALOG_LABEL_OPERATOR_DIVISION='/'
} analog_operator_button_label_t;

typedef char simple_expression_t[MAX_EXPRESSION_LENGTH];

static const char ALL_BUILTIN_OPERATORS[] = {
  ANALOG_LABEL_OPERATOR_ADDITION, ANALOG_LABEL_OPERATOR_SUBTRACTION,
  ANALOG_LABEL_OPERATOR_MULTIPLICATION, ANALOG_LABEL_OPERATOR_DIVISION
};

// this really should be in a header file
bool validate_expression(simple_expression_t expression);
bool check_expression_index_validity(simple_expression_t expression, uint8_t index);
bool check_expression_start_validity(simple_expression_t expression);
bool check_expression_end_validity(simple_expression_t expression, uint8_t len);
bool check_expression_single_operator_validity(simple_expression_t expression, uint8_t len);

bool has_multiple_operators(simple_expression_t expression, uint8_t len);
char get_operator_in_expression(simple_expression_t expression);

void evaluate_simple_expression(simple_expression_t expression, char operator_[]);

void get_inputted_expression(simple_expression_t *expression, uint8_t *length);

analog_button_code_t get_current_pressed_numpad_button_code(void);
analog_button_code_t get_current_operator_button_code(void);

analog_numpad_button_label_t map_analog_numpad_button_code_to_label(analog_button_code_t btn_code);
analog_operator_button_label_t map_analog_operator_button_code_to_label(analog_button_code_t btn_code);

__attribute__((always_inline)) static inline analog_button_code_t read_analog_numpad_set_1(void);
__attribute__((always_inline)) static inline analog_button_code_t read_analog_numpad_set_2(void);
__attribute__((always_inline)) static inline analog_button_code_t read_analog_operators(void);
__attribute__((always_inline)) static inline analog_button_code_t read_analog_calculate(void);

__attribute__((always_inline)) static inline void liquidcrystal_reset_write_screen(char *text);
__attribute__((always_inline)) static inline void liquidcrystal_write_screen_bottom(char *text);
__attribute__((always_inline)) static inline void liquidcrystal_write_screen_top(char *text);

void write_eval_expr_to_lcd(double calculated_expression);

// ***

__attribute__((always_inline)) static inline analog_button_code_t read_analog_numpad_set_1(void) {
  return (analog_button_code_t)(analogRead(ANALOG_NUMPAD_PIN_SET_1));
}

__attribute__((always_inline)) static inline analog_button_code_t read_analog_numpad_set_2(void) {
  return (analog_button_code_t)(analogRead(ANALOG_NUMPAD_PIN_SET_2));
}

__attribute__((always_inline)) static inline analog_button_code_t read_analog_operators(void) {
  return (analog_button_code_t)(analogRead(ANALOG_OPERATORS_PIN));
}

__attribute__((always_inline)) static inline analog_button_code_t read_analog_calculate(void) {
  return (analog_button_code_t)(analogRead(ANALOG_CALCULATE_FINAL_PIN));
}

__attribute__((always_inline)) static inline void liquidcrystal_reset_write_screen(char *text) {
  lcd.clear();
  liquidcrystal_write_screen_top(text);
}

__attribute__((always_inline)) static inline void liquidcrystal_write_screen_top(char *text) {
  lcd.setCursor(0, 0); lcd.print(text);
}

__attribute__((always_inline)) static inline void liquidcrystal_write_screen_bottom(char *text) {
  lcd.setCursor(0, 1); lcd.print(text);
}

void write_eval_expr_to_lcd(double calculated_expression) {
  simple_expression_t evaluated_expression = "\0"; // cannot use `snprintf` for float
  simple_expression_t final_expr_formatted = "\0"; // cannot use `snprintf` for float
  dtostrf(calculated_expression, 10,
           10, evaluated_expression);
  Serial.println(calculated_expression);
  snprintf(final_expr_formatted, sizeof(evaluated_expression),
           "=%s", evaluated_expression);

  liquidcrystal_write_screen_bottom(final_expr_formatted);
}

analog_numpad_button_label_t map_analog_numpad_button_code_to_label(analog_button_code_t btn_code) {
  switch (btn_code) {
    case ANALOG_CODE_BUTTON_0: return ANALOG_LABEL_BUTTON_0;
    case ANALOG_CODE_BUTTON_1: return ANALOG_LABEL_BUTTON_1;
    case ANALOG_CODE_BUTTON_2: return ANALOG_LABEL_BUTTON_2;
    case ANALOG_CODE_BUTTON_3: return ANALOG_LABEL_BUTTON_3;
    case ANALOG_CODE_BUTTON_4: return ANALOG_LABEL_BUTTON_4;
    case ANALOG_CODE_BUTTON_5: return ANALOG_LABEL_BUTTON_5;
    case ANALOG_CODE_BUTTON_6: return ANALOG_LABEL_BUTTON_6;
    case ANALOG_CODE_BUTTON_7: return ANALOG_LABEL_BUTTON_7;
    case ANALOG_CODE_BUTTON_8: return ANALOG_LABEL_BUTTON_8;
    case ANALOG_CODE_BUTTON_9: return ANALOG_LABEL_BUTTON_9;
    default: return ANALOG_LABEL_BUTTON_UNKNOWN;
  }
}

analog_operator_button_label_t map_analog_operator_button_code_to_label(analog_button_code_t btn_code) {
  switch(btn_code) {
    case ANALOG_CODE_BUTTON_ADDITION: return ANALOG_LABEL_OPERATOR_ADDITION;
    case ANALOG_CODE_BUTTON_SUBTRACTION: return ANALOG_LABEL_OPERATOR_SUBTRACTION;
    case ANALOG_CODE_BUTTON_MULTIPLICATION: return ANALOG_LABEL_OPERATOR_MULTIPLICATION;
    case ANALOG_CODE_BUTTON_DIVISION: return ANALOG_LABEL_OPERATOR_DIVISION;
  }
}

analog_button_code_t get_current_operator_button_code(void) {
  const analog_button_code_t ANALOG_READ_OP = read_analog_operators();

  if (ANALOG_READ_OP != ANALOG_OPERATORS_DEFAULT_VAL) {
    return ANALOG_READ_OP;
  } else {
    return ANALOG_NO_BUTTON_PRESSED;
  }
}

analog_button_code_t get_current_pressed_numpad_button_code(void) {
  const analog_button_code_t ANALOG_READ_NP_SET_1 = read_analog_numpad_set_1();
  const analog_button_code_t ANALOG_READ_NP_SET_2 = read_analog_numpad_set_2();

  if (ANALOG_READ_NP_SET_1 != ANALOG_NUMPAD_SET_1_DEFAULT_VAL) {
    return ANALOG_READ_NP_SET_1;
  } else if (ANALOG_READ_NP_SET_2 != ANALOG_NUMPAD_SET_2_DEFAULT_VAL) {
    return ANALOG_READ_NP_SET_2;
  } else {
    return ANALOG_NO_BUTTON_PRESSED;
  }
}

// calculator logic

bool check_expression_index_validity(simple_expression_t expression, uint8_t index) {
    for (uint8_t i=0; i <= sizeof(ALL_BUILTIN_OPERATORS); i++) {
        if (expression[index] == ALL_BUILTIN_OPERATORS[i]) return false;
    }
    return true;
}

bool check_expression_start_validity(simple_expression_t expression) {
    return check_expression_index_validity(expression, 0);
}

bool check_expression_end_validity(simple_expression_t expression, uint8_t len) {
    return check_expression_index_validity(expression, len - 1);
}

bool check_expression_single_operator_validity(simple_expression_t expression, uint8_t len) {
    for (uint8_t i = 0; i < len - 1; i++) {
        if (strchr(ALL_BUILTIN_OPERATORS, expression[i]) != NULL
            && strchr(ALL_BUILTIN_OPERATORS, expression[i + 1]) != NULL) {
            return false;
        }
    }
    return true;
}

bool validate_expression(simple_expression_t expression) {
  uint8_t expr_length = (uint8_t)strlen(expression);

  if (expr_length == 0) return false;
  if (!check_expression_start_validity(expression)) return false;
  if (!check_expression_end_validity(expression, expr_length)) return false;
  if (!check_expression_single_operator_validity(expression, expr_length)) return false;
  if (has_multiple_operators(expression, expr_length)) return false;

  return true;
}

void evaluate_simple_expression(simple_expression_t expression, char operator_[]) {
  if (!validate_expression(expression)) {
  	liquidcrystal_write_screen_bottom("Invalid Expr."); return;
  };

  liquidcrystal_write_screen_top(expression);

  char *split_expression = strtok(expression, operator_);
  uint32_t first_number = atoi(split_expression);
  split_expression = strtok(NULL, operator_);
  uint32_t second_number = atoi(split_expression);

  switch(*operator_) {
  	case ANALOG_LABEL_OPERATOR_ADDITION:
      write_eval_expr_to_lcd(first_number + second_number);
      break;
  	case ANALOG_LABEL_OPERATOR_SUBTRACTION:
      write_eval_expr_to_lcd(first_number - second_number);
      break;
  	case ANALOG_LABEL_OPERATOR_MULTIPLICATION:
      write_eval_expr_to_lcd(first_number * second_number);
      break;
  	case ANALOG_LABEL_OPERATOR_DIVISION:
      if (second_number != 0){
      	write_eval_expr_to_lcd((double)first_number / (double)second_number);
      	break;
      } else {
      	liquidcrystal_write_screen_bottom("ZeroDivisionErr");
      	break;
      }
  	default:
  		liquidcrystal_write_screen_bottom("Invalid Expr.");
  	}
}

bool has_multiple_operators(simple_expression_t expression, uint8_t len) {
    int operator_count = 0;
    for (uint32_t i = 0; i < len; i++) {
        if (strchr(ALL_BUILTIN_OPERATORS, expression[i]) != NULL) {
            operator_count++;
        }
    }
    return operator_count > 1;
}

void get_inputted_expression(simple_expression_t *expression, uint8_t *length) {
  analog_button_code_t current_np_pressed_button = get_current_pressed_numpad_button_code();
  analog_button_code_t current_op_pressed_button = get_current_operator_button_code();

  if (current_np_pressed_button != ANALOG_NO_BUTTON_PRESSED) {
    analog_numpad_button_label_t button_label = map_analog_numpad_button_code_to_label(
      current_np_pressed_button
    );
    char button_label_as_char = DIG2CHAR(button_label);

    if (*length < sizeof(simple_expression_t)) {
      (*expression)[(*length)++] = button_label_as_char;
      liquidcrystal_reset_write_screen(*expression);
    }
  } else if (current_op_pressed_button != ANALOG_NO_BUTTON_PRESSED) {
    char button_label_as_char = (char)map_analog_operator_button_code_to_label(
      current_op_pressed_button
    );

    if (*length < sizeof(simple_expression_t)) {
      (*expression)[(*length)++] = button_label_as_char;
      liquidcrystal_reset_write_screen(*expression);
    }
  }
}

char get_operator_in_expression(simple_expression_t expression) {
  for (uint8_t i=0; i <= sizeof(expression); i++) {
    for (uint8_t j=0; j <= sizeof(ALL_BUILTIN_OPERATORS); j++) {
      if (expression[i] == ALL_BUILTIN_OPERATORS[j]) {
        return ALL_BUILTIN_OPERATORS[j];
      }
    }
  }
}

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
};

void loop() {
  static simple_expression_t expr = "\0";
  static uint8_t length = 0;

  if (read_analog_calculate() != ANALOG_CALCULATE_DEFAULT_VAL) {
    char operator_[2] = {get_operator_in_expression(expr), '\0'};
    evaluate_simple_expression(expr, operator_);
    length = 0;
    memset(expr, 0, sizeof(expr));
    delay(1000); // wait 1 second after result
  }

  get_inputted_expression(&expr, &length);

  delay(200);
}
