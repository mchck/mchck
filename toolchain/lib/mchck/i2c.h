enum i2c_stop {
        I2C_NOSTOP,
        I2C_STOP
};

enum i2c_rate {
        I2C_RATE_100 = 0,
        I2C_RATE_400,
        I2C_RATE_600,
        I2C_RATE_800,
        I2C_RATE_1000,
        I2C_RATE_1200,
        I2C_RATE_1500,
        I2C_RATE_2000,
        I2C_RATE_2400
};

enum i2c_result {
        I2C_RESULT_SUCCESS,
        I2C_RESULT_NACK
};

struct i2c_transaction;

typedef void (i2c_cb)(enum i2c_result result,
		struct i2c_transaction *transaction
	);

struct i2c_transaction {
	// address & R/!W bit
	uint8_t address : 7;
	uint8_t read : 1;

	// whether to end with stop condition
	enum i2c_stop stop;

	// buffer and length
	uint8_t *buffer;
	size_t length;

	// callback info
	i2c_cb *cb;
	void *cbdata;

	// linked list pointer
	struct i2c_transaction *next;
};

void
i2c_init(enum i2c_rate rate);

void
i2c_queue(struct i2c_transaction *transaction);

// these are macros to get a new struct each time they're invoked
// this means they consume some memory!
#define _i2c_queue_read(var, addr, stop, buffer, length, cb, cbdata) {\
		static struct i2c_transaction var = { addr, 1, stop, buffer, length, cb, cbdata, NULL };\
		i2c_queue(&var);\
	}

#define _i2c_queue_write(var, addr, stop, buffer, length, cb, cbdata) {\
		static struct i2c_transaction var = { addr, 0, stop, buffer, length, cb, cbdata, NULL };\
		i2c_queue(&var);\
	}

#define i2c_queue_read(addr, stop, buffer, length, cb, cbdata) \
	_i2c_queue_read(_CONCAT(i2c_transaction_, __COUNTER__), addr, stop, buffer, length, cb, cbdata)

#define i2c_queue_write(addr, stop, buffer, length, cb, cbdata) \
	_i2c_queue_write(_CONCAT(i2c_transaction_, __COUNTER__), addr, stop, buffer, length, cb, cbdata)
