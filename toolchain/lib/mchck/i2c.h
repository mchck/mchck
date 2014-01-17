enum i2c_stop {
	I2C_NOSTOP,
	I2C_STOP,
};

enum i2c_direction {
	I2C_READ,
	I2C_WRITE,
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
	I2C_RATE_2400,
};

enum i2c_result {
	I2C_RESULT_SUCCESS,
	I2C_RESULT_NACK,
};

struct i2c_transaction;

typedef void	(i2c_cb)(enum i2c_result result,
		    struct i2c_transaction *transaction
		    );

struct i2c_transaction {
	// destination address and whether read or write 
	uint8_t address;
	enum i2c_direction direction;

	// whether to end with stop condition
	enum i2c_stop stop;

	// data buffer and length
	uint8_t *buffer;
	size_t length;

	// callback info
	i2c_cb *cb;
	void *cbdata;

	// linked list pointer
	struct i2c_transaction *next;
};

void	i2c_init(enum i2c_rate rate);

void	i2c_queue(struct i2c_transaction *transaction);
