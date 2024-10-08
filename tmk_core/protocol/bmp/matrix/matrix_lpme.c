
#include "lpme.h"
#include "bmp_matrix.h"
#include "quantum.h"
#include "apidef.h"
#include "i2c.h"

static void                    init_row2col(void);
static void                    init_col2row(void);
static uint32_t                get_device_row(void);
static uint32_t                get_device_col(void);
static uint32_t                scan_row2col(matrix_row_t *);
static uint32_t                scan_col2row(matrix_row_t *);
extern const bmp_matrix_func_t matrix_func_row2col;
extern const bmp_matrix_func_t matrix_func_col2row;

const bmp_matrix_func_t matrix_func_row2col_lpme = {init_row2col, get_device_row, get_device_col, scan_row2col};
const bmp_matrix_func_t matrix_func_col2row_lpme = {init_col2row, get_device_row, get_device_col, scan_col2row};

static uint8_t       lpme_row_num, lpme_col_num;
static uint8_t       pin_matrix_row_num, pin_matrix_col_num;
static lpme_config_t lpme_config;

static uint32_t get_device_row(void) {
    return lpme_row_num + pin_matrix_row_num;
}

static uint32_t get_device_col(void) {
    return pin_matrix_col_num;
}

static uint32_t get_lpme_row_num(void) {
    const bmp_api_config_t *config = BMPAPI->app.get_config();
    uint32_t                row_pin_idx;

    for (row_pin_idx = 0; row_pin_idx < sizeof(config->matrix.row_pins); row_pin_idx++) {
        if (config->matrix.row_pins[row_pin_idx] == 0) {
            break;
        }
    }

    if (row_pin_idx < config->matrix.device_rows) {
        return 0;
    } else {
        return row_pin_idx - config->matrix.device_rows;
    }
}

static uint32_t get_lpme_col_num(void) {
    const bmp_api_config_t *config = BMPAPI->app.get_config();
    uint32_t                col_pin_idx;

    for (col_pin_idx = 0; col_pin_idx < sizeof(config->matrix.col_pins); col_pin_idx++) {
        if (config->matrix.col_pins[col_pin_idx] == 0) {
            break;
        }
    }

    if (col_pin_idx < config->matrix.device_cols) {
        return 0;
    } else {
        return col_pin_idx - config->matrix.device_cols;
    }
}

//
//// col2row matrix
//
static void init_col2row(void) {
    const bmp_api_config_t *config = BMPAPI->app.get_config();

    matrix_func_col2row.init();

    pin_matrix_row_num = config->matrix.device_rows;
    pin_matrix_col_num = config->matrix.device_cols;
    lpme_row_num       = get_lpme_row_num();
    lpme_col_num       = get_lpme_col_num();

    lpme_config.row_num = lpme_row_num;
    lpme_config.col_num = lpme_col_num;

    for (int r = 0; r < lpme_row_num; r++) {
        lpme_config.row_pins[r] = config->matrix.row_pins[r + pin_matrix_row_num];
    }

    for (int c = 0; c < lpme_col_num; c++) {
        lpme_config.col_pins[c] = config->matrix.col_pins[c + pin_matrix_col_num];
    }

    lpme_init(&lpme_config);
}

static uint32_t scan_col2row(matrix_row_t *matrix_raw) {
    const bmp_api_config_t *config = BMPAPI->app.get_config();
    uint32_t                change = matrix_func_col2row.scan(matrix_raw);

    uint8_t matrix_offset_slave = config->matrix.is_left_hand ? config->matrix.device_rows : 0;
    change |= lpme_scan(&lpme_config, &matrix_raw[matrix_offset_slave], change);

    return change;
}

//
//// row2col matrix
//
static void init_row2col(void) {
    const bmp_api_config_t *config = BMPAPI->app.get_config();

    matrix_func_row2col.init();

    pin_matrix_row_num = config->matrix.device_rows;
    pin_matrix_col_num = config->matrix.device_cols;
    lpme_row_num       = get_lpme_row_num();
    lpme_col_num       = get_lpme_col_num();

    // swap row/col of matrix setting

    lpme_config.row_num = lpme_col_num;
    lpme_config.col_num = lpme_row_num;

    for (int r = 0; r < lpme_col_num; r++) {
        lpme_config.row_pins[r] = config->matrix.col_pins[r + pin_matrix_col_num];
    }

    for (int c = 0; c < lpme_row_num; c++) {
        lpme_config.col_pins[c] = config->matrix.row_pins[c + pin_matrix_row_num];
    }

    lpme_init(&lpme_config);
}

static uint32_t scan_row2col(matrix_row_t *matrix_raw) {
    const bmp_api_config_t *config = BMPAPI->app.get_config();
    uint32_t                change = matrix_func_row2col.scan(matrix_raw);

    uint8_t matrix_offset_slave = config->matrix.is_left_hand ? config->matrix.device_rows : 0;
    change |= lpme_scan(&lpme_config, &matrix_raw[matrix_offset_slave], change);

    // swap row and col of split half

    // len = max(row, col)
    uint8_t len = lpme_config.row_num > lpme_config.col_num ? lpme_config.row_num : lpme_config.col_num;

    for (int ridx = 0; ridx < len; ridx++) {
        for (int cidx = ridx + 1; cidx < len; cidx++) {
            bool m_rc = matrix_raw[ridx + matrix_offset_slave] & (1 << cidx) ? true : false;
            bool m_cr = matrix_raw[cidx + matrix_offset_slave] & (1 << ridx) ? true : false;

            // set m_rc to m_cr
            if (m_rc) {
                matrix_raw[cidx + matrix_offset_slave] |= (1 << ridx);
            } else {
                matrix_raw[cidx + matrix_offset_slave] &= ~(1 << ridx);
            }

            // set m_cr to m_rc
            if (m_cr) {
                matrix_raw[ridx + matrix_offset_slave] |= (1 << cidx);
            } else {
                matrix_raw[ridx + matrix_offset_slave] &= ~(1 << cidx);
            }
        }
    }
    return change;
}
