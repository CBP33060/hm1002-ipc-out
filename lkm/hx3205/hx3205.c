#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

// #define DEBUG

#define ADC_15

#define HX3205_ALS_ENABLE_REG				0x01
#define HX3205_PD_SEL_REG					0x02
#define HX3205_INT_CTRL0_REG				0x05
#define HX3205_INT_CTRL1_REG				0x06
#define HX3205_INT_CTRL2_REG				0x07
#define HX3205_PRF_CTRL0_REG				0x0F
#define HX3205_PRF_CTRL1_REG				0x10
#define HX3205_PRF_CTRL2_REG				0x11
#define HX3205_UPPER_THRESHOLD_L_REG		0x12
#define HX3205_UPPER_THRESHOLD_H_REG		0x13
#define HX3205_LOWER_THRESHOLD_L_REG		0x14
#define HX3205_LOWER_THRESHOLD_H_REG		0x15
#define HX3205_ADCGAIN_ADC_RST_CTRL_REG		0x16
#define HX3205_CURGAIN_CTRL_REG				0x17
#define HX3205_ALS_INTERVAL_REG				0x18
#define HX3205_ADC_INTCAPSEL_ALS_REG		0x19
#define HX3205_ADC_RANGE_REG				0xC1
#define HX3205_ADC_MODE_REG					0xC7

//readonly
#define HX3205_MANUFAC_ID_REG			0x00
#define HX3205_DATA_0_REG				0xA6
#define HX3205_DATA_1_REG				0xA6

struct hx3205_private {
	struct mutex mutex;
	unsigned short upper_threshold;
	unsigned short lower_threshold;
	int int_gpio_num;
	int int_gpio_irq_num;
	wait_queue_head_t r_wait;
	struct i2c_client *i2c_client;
	int timeout;
};

struct kset *als_kset;
static struct hx3205_private *hx3205_data;
static unsigned short g_upper_threshold = 5000;
static unsigned short g_lower_threshold = 1000;
static struct delayed_work delay_work;

module_param(g_upper_threshold, ushort, S_IRUGO | S_IWUSR);
module_param(g_lower_threshold, ushort, S_IRUGO | S_IWUSR);

static int hx3205_read(struct i2c_client *i2c_client);

static void delay_work_func(struct work_struct *work)
{
	static int i = 0;
	hx3205_read(hx3205_data->i2c_client);
	if(i++ < 20)
    	schedule_delayed_work(&delay_work, 0.2 * HZ);
}
static uint8_t agc_step = 4;    // valid value from 1 to 10
static uint8_t agc_step_old = 4;
static int hx3205_init(struct i2c_client *i2c_client)
{
	int ret = 0;
	unsigned char data0,data1;
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);

	data->upper_threshold = g_upper_threshold;
	data->lower_threshold = g_lower_threshold;
	i2c_smbus_write_byte_data(i2c_client, 0x01, 0x31); // als off
	i2c_smbus_write_byte_data(i2c_client, 0x02, 0x35); //37
	i2c_smbus_write_byte_data(i2c_client, 0x03, 0x01);
	i2c_smbus_write_byte_data(i2c_client, 0x04, 0x00);
	i2c_smbus_write_byte_data(i2c_client, 0x1c, 0x00);
	i2c_smbus_write_byte_data(i2c_client, 0x05, 0xff);
	i2c_smbus_write_byte_data(i2c_client, 0x06, 0x41);
	i2c_smbus_write_byte_data(i2c_client, 0x07, 0x40); // bit7 als_rdy
	// i2c_smbus_write_byte_data(i2c_client, 0x08, 0xc1);
	i2c_smbus_write_byte_data(i2c_client, 0x09, 0x40);
	// i2c_smbus_write_byte_data(i2c_client, 0x0A,0x00);
	// i2c_smbus_write_byte_data(i2c_client, 0x0e,0x04);
	
	//16:0x00 0x40 0x03
	//15:0x00 0x32 0x01 
	//14:0x00 0x99 0x00
	//13:0x80 0x4c 0x00
	//12:0x40 0x26 0x00
	
	#if defined ADC_16
		i2c_smbus_write_byte_data(i2c_client, 0x0f, 0x00); //prf_num_i2c[7:0]
		i2c_smbus_write_byte_data(i2c_client, 0x10, 0x40); //prf_num_i2c[15:8]
		i2c_smbus_write_byte_data(i2c_client, 0x11, 0x03); //prf_num_i2c[21:16]
	#elif defined ADC_15
		i2c_smbus_write_byte_data(i2c_client, 0x0f, 0x00); //prf_num_i2c[7:0]
		i2c_smbus_write_byte_data(i2c_client, 0x10, 0x32); //prf_num_i2c[15:8]
		i2c_smbus_write_byte_data(i2c_client, 0x11, 0x01); //prf_num_i2c[21:16]
	#elif defined ADC_14
		i2c_smbus_write_byte_data(i2c_client, 0x0f, 0x00); //prf_num_i2c[7:0]
		i2c_smbus_write_byte_data(i2c_client, 0x10, 0x99); //prf_num_i2c[15:8]
		i2c_smbus_write_byte_data(i2c_client, 0x11, 0x00); //prf_num_i2c[21:16]
	#elif defined ADC_13
		i2c_smbus_write_byte_data(i2c_client, 0x0f, 0x80); //prf_num_i2c[7:0]
		i2c_smbus_write_byte_data(i2c_client, 0x10, 0x4C); //prf_num_i2c[15:8]
		i2c_smbus_write_byte_data(i2c_client, 0x11, 0x00); //prf_num_i2c[21:16]
	#elif defined ADC_12
		i2c_smbus_write_byte_data(i2c_client, 0x0f, 0x40); //prf_num_i2c[7:0]
		i2c_smbus_write_byte_data(i2c_client, 0x10, 0x26); //prf_num_i2c[15:8]
		i2c_smbus_write_byte_data(i2c_client, 0x11, 0x00); //prf_num_i2c[21:16]
	#endif

	switch(agc_step)
	{
		case 7:
			i2c_smbus_write_byte_data(i2c_client, 0x16,0x33); i2c_smbus_write_byte_data(i2c_client, 0x17,0x08);	   // 2048x, adc 8x, amp 256x
			break;
		case 6:
			i2c_smbus_write_byte_data(i2c_client, 0x16, 0x23); i2c_smbus_write_byte_data(i2c_client, 0x17, 0x08);	   // 1024x, adc 4x, amp 256x
			break;
		case 5:
			i2c_smbus_write_byte_data(i2c_client, 0x16,0x13); i2c_smbus_write_byte_data(i2c_client, 0x17,0x18);	   // 256x, adc 2x, amp 128x
			break;
		case 4:
			i2c_smbus_write_byte_data(i2c_client, 0x16,0x03); i2c_smbus_write_byte_data(i2c_client, 0x17,0x28);	   // 64x, adc 1x, amp 64x
			break;
		case 3:
			i2c_smbus_write_byte_data(i2c_client, 0x16,0x03); i2c_smbus_write_byte_data(i2c_client, 0x17,0x48);	   // 16x, adc 1x, amp 16x
			break;
		case 2:
			i2c_smbus_write_byte_data(i2c_client, 0x16,0x03); i2c_smbus_write_byte_data(i2c_client, 0x17,0x68);	   // 4x, adc 1x, amp 4x
			break;
		case 1:
			i2c_smbus_write_byte_data(i2c_client, 0x16,0x03); i2c_smbus_write_byte_data(i2c_client, 0x17,0x88);	   // 1x, adc 1x, amp 1x
			break;
	}
	// i2c_smbus_write_byte_data(i2c_client, 0x16,0x33); i2c_smbus_write_byte_data(i2c_client, 0x17,0x08);	   // 2048x, adc 8x, amp 256x
	//i2c_smbus_write_byte_data(i2c_client, 0x16, 0x23); i2c_smbus_write_byte_data(i2c_client, 0x17, 0x08);	   // 1024x, adc 4x, amp 256x
	// i2c_smbus_write_byte_data(i2c_client, 0x16,0x13); i2c_smbus_write_byte_data(i2c_client, 0x17,0x18);	   // 256x, adc 2x, amp 128x
	// i2c_smbus_write_byte_data(i2c_client, 0x16,0x03); i2c_smbus_write_byte_data(i2c_client, 0x17,0x28);	   // 64x, adc 1x, amp 64x
	// i2c_smbus_write_byte_data(i2c_client, 0x16,0x03); i2c_smbus_write_byte_data(i2c_client, 0x17,0x48);	   // 16x, adc 1x, amp 16x
	// i2c_smbus_write_byte_data(i2c_client, 0x16,0x03); i2c_smbus_write_byte_data(i2c_client, 0x17,0x68);	   // 4x, adc 1x, amp 4x
	// i2c_smbus_write_byte_data(i2c_client, 0x16,0x03); i2c_smbus_write_byte_data(i2c_client, 0x17,0x88);	   // 1x, adc 1x, amp 1x
	i2c_smbus_write_byte_data(i2c_client, 0x18, 0x00);	// als3_interval_i2c
	i2c_smbus_write_byte_data(i2c_client, 0x19, 0xff);	// ff 7f
	i2c_smbus_write_byte_data(i2c_client, 0x1d, 0x30);	// 30   38
	i2c_smbus_write_byte_data(i2c_client, 0x1e, 0x23); // ps_tck_num[7:0]
	i2c_smbus_write_byte_data(i2c_client, 0x1f, 0x60); // bit5 ps1 sel 0:ps0, 1:ps0-ps1 ; bit4:0 ps_tck_num[12:8]
	i2c_smbus_write_byte_data(i2c_client, 0x20, 0x40); i2c_smbus_write_byte_data(i2c_client, 0x21, 0x02);// single pulse
	// i2c_smbus_write_byte_data(i2c_client, 0x20,0x40); i2c_smbus_write_byte_data(i2c_client, 0x21,0x03);// two pulse
	// i2c_smbus_write_byte_data(i2c_client, 0x20,0x10); i2c_smbus_write_byte_data(i2c_client, 0x21,0x05);// four pulse
	// i2c_smbus_write_byte_data(i2c_client, 0x20,0x08); i2c_smbus_write_byte_data(i2c_client, 0x21,0x09);// eight pulse
	// i2c_smbus_write_byte_data(i2c_client, 0xc0,0x17);
	i2c_smbus_write_byte_data(i2c_client, 0xc1, 0x55); // bit7: ADC range sel; =1 1.5X, =0 1X; bit6: CUAOFFEN
	i2c_smbus_write_byte_data(i2c_client, 0xc2, 0x83); // bit5:4 CLK divider,00 div1 01 div2 10 div4 11 div8
	i2c_smbus_write_byte_data(i2c_client, 0xc4, 0xff); // bit4:0 led drv ,e0:100mA, 06:21.875, 05:18.75
	i2c_smbus_write_byte_data(i2c_client, 0xc6, 0x34); // bit5:4 pd_gain 00 1/4X ; 01 1/2X ; 11 1X;
	i2c_smbus_write_byte_data(i2c_client, 0xc7, 0xa0); // bit7:ADCNRZEN bit5:ADCINTOPOFFEN

	//16:0xc1 15:0xb1 14:0xa1 13:0x91 12:0x81
	#if defined ADC_16
		i2c_smbus_write_byte_data(i2c_client, 0x01, 0xc1); // als on
	#elif defined ADC_15
		i2c_smbus_write_byte_data(i2c_client, 0x01, 0xb1); // als on
	#elif defined ADC_14
		i2c_smbus_write_byte_data(i2c_client, 0x01, 0xa1); // als on
	#elif defined ADC_13
		i2c_smbus_write_byte_data(i2c_client, 0x01, 0x91); // als on
	#elif defined ADC_12
		i2c_smbus_write_byte_data(i2c_client, 0x01, 0x81); // als on
	#endif

    // data1 = ((data->upper_threshold & 0xff00) >> 8);
    // data0 = (data->upper_threshold & 0x00ff);
    // i2c_smbus_write_byte_data(i2c_client, HX3205_UPPER_THRESHOLD_L_REG, data0);
    // i2c_smbus_write_byte_data(i2c_client, HX3205_UPPER_THRESHOLD_H_REG, data1);

	#ifdef DEBUG
	dev_err(&i2c_client->dev,"HX3205_UPPER_THRESHOLD_H_REG0:0x%x HX3205_UPPER_THRESHOLD_H_REG1:0x%x HX3205_UPPER_THRESHOLD_H_REG:0x%x\n",data0,data1,data0 | (data1 << 8));
	#endif

    // data1 = ((data->lower_threshold & 0xff00) >> 8);
    // data0 = (data->lower_threshold & 0x00ff);
    // i2c_smbus_write_byte_data(i2c_client, HX3205_LOWER_THRESHOLD_L_REG, data0);
    // i2c_smbus_write_byte_data(i2c_client, HX3205_LOWER_THRESHOLD_H_REG, data1);

	#ifdef DEBUG
	dev_err(&i2c_client->dev,"HX3205_LOWER_THRESHOLD_L_REG0:0x%x HX3205_LOWER_THRESHOLD_L_REG1:0x%x HX3205_LOWER_THRESHOLD_L_REG:0x%x\n",data0,data1,data0 | (data1 << 8));
	#endif

	i2c_smbus_write_byte_data(i2c_client, 0x08, 0x0d);
	mdelay(1);
	i2c_smbus_write_byte_data(i2c_client, 0x08, 0x09);

#ifdef DEBUG
	INIT_DELAYED_WORK(&delay_work, delay_work_func);
    schedule_delayed_work(&delay_work, 0.1 * HZ);
#endif

	return ret;
}

static int read_hx3205_als1(struct i2c_client *i2c_client)
{
	uint8_t  databuf[2];
	int data;

	databuf[1] = i2c_smbus_read_byte_data(i2c_client, 0xA4);    // lock data
	databuf[1] = i2c_smbus_read_byte_data(i2c_client, 0xA4);    // addr14, bit [7:4]  	
	databuf[0] = i2c_smbus_read_byte_data(i2c_client, 0xA5);    // addr13, bit [15:8]  

	if(databuf[1] < 0 || databuf[0] < 0) {
		printk("read_hx3205_als1 failed\n");
		return -1;
	}

	data = ((databuf[0] << 8) | databuf[1]);
	return data;
}

static int read_hx3205_als2(struct i2c_client *i2c_client)
{
	uint8_t  databuf[2];
	int data;

	databuf[1] = i2c_smbus_read_byte_data(i2c_client, 0xA6);    // lock data    
	databuf[1] = i2c_smbus_read_byte_data(i2c_client, 0xA6);    // addr14, bit [7:4]
	databuf[0] = i2c_smbus_read_byte_data(i2c_client, 0xA7);    // addr13, bit [15:8]    

	if(databuf[1] < 0 || databuf[0] < 0) {
		printk("read_hx3205_als2 failed\n");
		return -1;
	}

	data = ((databuf[0] << 8) | databuf[1]);
	return data;
}

static int read_hx3205_als3(struct i2c_client *i2c_client)
{
	uint8_t  databuf[2];
	int data;

	databuf[1] = i2c_smbus_read_byte_data(i2c_client, 0xA8);    // lock data   
	databuf[1] = i2c_smbus_read_byte_data(i2c_client, 0xA8);    // addr14, bit [7:4]
	databuf[0] = i2c_smbus_read_byte_data(i2c_client, 0xA9);    // addr13, bit [15:8]    

	if(databuf[1] < 0 || databuf[0] < 0) {
		printk("read_hx3205_als3 failed\n");
		return -1;
	}

	data = ((databuf[0] << 8) | databuf[1]);
	return data;
}
#define COATING_VERSION

#define A_RATIO_NO_COVER     60
#define A_CORR_NO_COVER      21
#define C_RATIO_NO_COVER     162
#define A_RATIO_COVER        40
#define C_RATIO_COVER        117
#define A_CORR_COVER         34 

#define RATIO_STOP_ALS2      1000
#define RATIO_STOP_MULT      30
#define RATIO_FLT_ALPHA      3
#define ALS_TIMER            100     // unit is ms
#define ALS3_PERIOD          100
#define ALS1_TEMP_COE        10      // 10x
#define ALS2_TEMP_COE        25      // 10x

#define AGC_OSR             0

static int als1 = 0;
static int als1_pre = 0;
static int als1_pre2 = 0;
static int als2 = 0;
static int als2_pre = 0;
static int als3 = 0;
static uint32_t als_ir_ratio_128x = 384;        // als ir ratio. 128*3
static uint8_t  new_sw1 = 1;
// static uint16_t cwf_base_ratio = 3; //c光 als2/als1*1000, C_RATIO_COVER
static uint16_t cwf_base_ratio_thl = 30; // C_RATIO_COVER_C= C_RATIO_COVER*10 --> start
static uint16_t cwf_base_ratio_thh = 80; // C_RATIO_COVER_D= C_RATIO_COVER*30 --> D65的区间start
static uint16_t cwf_base_ratio_thh_d = 150; // C_RATIO_COVER_D= C_RATIO_COVER*30 --> D65的区间end
static uint16_t cwf_base_ratio_thh_a = 200; // C_RATIO_COVER_A= C_RATIO_COVER*60 -->A 区间
//16:50000 15:30000 14:12000 13:6000 12:3000
#if defined ADC_16
	static uint16_t agc_thh = 50000;  // 78% of FS
#elif defined ADC_15
	static uint16_t agc_thh = 30000;  // 78% of FS
#elif defined ADC_14
	static uint16_t agc_thh = 12000;  // 78% of FS
#elif defined ADC_13
	static uint16_t agc_thh = 6000;  // 78% of FS
#elif defined ADC_12
	static uint16_t agc_thh = 3000;  // 78% of FS
#endif
static uint16_t als_adj_set = 800; // d65 lihgt coe
static uint16_t als_adj_set2 = 900; // a lihgt coe
static uint16_t als_adj_pre = 1000; // 
static uint16_t als_sen_mlux =1000;
static uint16_t als1_buf[4] = { 0 };
static uint16_t als2_buf[4] = { 0 };
static uint8_t  als_avg_en = 0;
static uint32_t als_normalized_pre = 0;
static uint32_t avg_cnt = 0;
static uint8_t  agc_step_max = 7;
static volatile uint8_t chip_id = 0;
static volatile uint8_t chip_sleep = 0;
static volatile uint32_t report_tim = 0;
// static uint32_t delay_count = 0;

// static uint8_t gain_16reg = 0;
// static uint8_t gain_17reg = 0;
// static uint8_t gain_c8reg = 0;

// static uint8_t agc_step = 6;    // valid value from 1 to 10
// static uint8_t agc_step_old = 6;
static uint8_t agc_lock_flag = 0;
static uint8_t agc_step_pre = 6;

static uint8_t agc_osr = 0;
// static uint8_t agc_dec = 0;
// static uint8_t agc_inc = 0;

static void hx3205_agc_config(struct i2c_client *i2c_client,uint8_t agc_step)
{
	//    agc_step = 6;
	if (agc_step <= 1) {
		i2c_smbus_write_byte_data(i2c_client, 0x16, 0x03);
		i2c_smbus_write_byte_data(i2c_client, 0x17, 0x88);	   // 1x, adc 1x, amp 1x
	} else if (agc_step == 2) {
		i2c_smbus_write_byte_data(i2c_client, 0x16, 0x03);
		i2c_smbus_write_byte_data(i2c_client, 0x17, 0x68);	   // 4x, adc 1x, amp 4x
	} else if (agc_step == 3) {
		i2c_smbus_write_byte_data(i2c_client, 0x16, 0x03);
		i2c_smbus_write_byte_data(i2c_client, 0x17, 0x48);	   // 16x, adc 1x, amp 16x
	} else if (agc_step == 4) {
		i2c_smbus_write_byte_data(i2c_client, 0x16, 0x03);
		i2c_smbus_write_byte_data(i2c_client, 0x17, 0x28);	   // 64x, adc 1x, amp 64x
	} else if (agc_step == 5) {
		i2c_smbus_write_byte_data(i2c_client, 0x16, 0x13);
		i2c_smbus_write_byte_data(i2c_client, 0x17, 0x18);	   // 256x, adc 2x, amp 128x
	} else if (agc_step == 6) {
		i2c_smbus_write_byte_data(i2c_client, 0x16, 0x23);
		i2c_smbus_write_byte_data(i2c_client, 0x17, 0x08);	   // 1024x, adc 4x, amp 256x
	} else if (agc_step >= 7) {
		i2c_smbus_write_byte_data(i2c_client, 0x16, 0x33);
		i2c_smbus_write_byte_data(i2c_client, 0x17, 0x08);	   // 2048x, adc 8x, amp 256x
	}
}

// static void HX3205_adjust_als(struct i2c_client *i2c_client,uint16_t *als, uint8_t e_alsline)
// {
// 	uint8_t alinebit2 = 0;
// 	uint32_t als_temp = 0;
// 	uint8_t k = 0;

// 	alinebit2 = (e_alsline >> 2) & 0x01;
// 	k = (e_alsline & 0x03) << 1;

// 	als_temp = *als*k;

// 	if (alinebit2 == 1)
// 	{

// 		*als += als_temp / 100;
// 	}
// 	else if (alinebit2 == 0)
// 	{

// 		*als -= als_temp / 100;
// 	}
// }

// static void getagc_hx3205(struct i2c_client *i2c_client)
// {
// 	uint8_t reg16;
// 	uint8_t reg17;
// 	reg16 = i2c_smbus_read_byte_data(i2c_client, 0x16);
// 	reg16 = (reg16 & 0xf0) >> 4;

// 	reg17 = i2c_smbus_read_byte_data(i2c_client, 0x17);
// 	reg17 = (reg17 & 0xf0) >> 4;

// 	//		SEGGER_RTT_printf(0,"reg16 = %d,\treg17 = %d,\t",reg16,reg17);

// 	if (reg16 == 0)
// 	{
// 		if (reg17 == 8)
// 		{
// 			agc_step = 1;
// 		}
// 		else if (reg17 == 6)
// 		{
// 			agc_step = 2;
// 		}
// 		else if (reg17 == 4)
// 		{
// 			agc_step = 3;
// 		}
// 		else if (reg17 == 2)
// 		{
// 			agc_step = 4;
// 		}
// 	}
// 	else if ((reg16 == 1) && (reg17 == 1))
// 	{
// 		agc_step = 5;
// 	}
// 	else if ((reg16 == 2) && (reg17 == 0))
// 	{
// 		agc_step = 6;
// 	}
// 	else if ((reg16 == 3) && (reg17 == 0))
// 	{
// 		agc_step = 7;
// 	}

// }

static void hx3205_agc(struct i2c_client *i2c_client,uint16_t als1, uint16_t als2)
{
	uint16_t temp = 0;

	temp = agc_thh >> 3;

	if (agc_osr > 0) {
		agc_osr--;
		return;
	}

	if ((als1 > agc_thh) || (als2 > agc_thh)) {      //80% max(65536)
		if (agc_step > 1) {
			agc_step = agc_step - 1;
		}
	}
	if (als1 < temp) {   //20% max(65536)
		if (agc_step < agc_step_max) {
			agc_step = agc_step + 1;
		}
	}
	// 		SEGGER_RTT_printf(0,"agc = %d, \r\n", agc_step);   
	//    hx3205_agc_config(i2c_client,agc_step);
	if (agc_step != agc_step_old) {
		#ifdef DEBUG 
		printk("agc_step_old = %d, agc_step = %d\r\n", agc_step_old, agc_step);
		#endif // DEBUG
		agc_step_old = agc_step;
		hx3205_agc_config(i2c_client,agc_step);
		agc_osr = AGC_OSR;
		avg_cnt = 0;
	}

}

static int read_hx3205_als(struct i2c_client *i2c_client)
{
	// uint8_t reg_cnt = 0;
	uint8_t  ii = 0;
	uint8_t  adj_update_flag = 0;
	uint32_t  als_ir_ratio = 0;      // times 100
	uint32_t  als_ir_ratio_temp = 0;
	uint32_t  als_ir_ratio_temp2 = 0;
	uint16_t als1_diff_abs = 0, als1_diff2_abs = 0;
	//    uint8_t  als_adj_ind = 0;
	uint16_t als_adj = 0;
	int als_normalized = 0;
	//    uint16_t als1_offset_comp = 0, als2_offset_comp = 0, als1_temp_comp = 0, als2_temp_comp = 0;
	uint32_t lux_10000x = 0;
	uint16_t als1_read_temp = 0;
	uint16_t als2_read_temp = 0;
	uint32_t als1_read_temp32 = 0;
	uint32_t als2_read_temp32 = 0;
	int lux = 0;

	avg_cnt = avg_cnt + 1;
	als1_pre2 = als1_pre;
	als1_pre = als1;
	als2_pre = als2;
	//		als1=	read_hx3205_als1(i2c_client);
	agc_step_pre = agc_step;

	if (als_avg_en == 1) {
		als1_read_temp = read_hx3205_als1(i2c_client);
		als2_read_temp = read_hx3205_als2(i2c_client);
		if(als1_read_temp < 0 || als2_read_temp < 0) {
			return -1;
		}
		als1_buf[3] = als1_buf[2];
		als1_buf[2] = als1_buf[1];
		als1_buf[1] = als1_buf[0];
		als1_buf[0] = als1_read_temp;

		als2_buf[3] = als2_buf[2];
		als2_buf[2] = als2_buf[1];
		als2_buf[1] = als2_buf[0];
		als2_buf[0] = als2_read_temp;

		if (avg_cnt > 5) {
			als1_read_temp32 = (als1_buf[0] + als1_buf[1] + als1_buf[2] + als1_buf[3]) >> 2;
			als2_read_temp32 = (als2_buf[0] + als2_buf[1] + als2_buf[2] + als2_buf[3]) >> 2;
			als1 = als1_read_temp32;
			als2 = als2_read_temp32;
		} else {
			als1 = als1_read_temp;
			als2 = als2_read_temp;
		}
	} else {
		als1 = read_hx3205_als1(i2c_client);
		als2 = read_hx3205_als2(i2c_client);

	}
	als3 = read_hx3205_als3(i2c_client);
	if(als1 < 0 || als2 < 0 || als3 < 0) {
		return -1;
	}

	als_ir_ratio_temp = als2 * 100 / als1;
	als_ir_ratio_temp2 = als2_pre * 100 / als1_pre;

	if (als1 > als1_pre) {
		als1_diff_abs = als1 - als1_pre;
	} else {
		als1_diff_abs = als1_pre - als1;
	}
	if (als1 > als1_pre2) {
		als1_diff2_abs = als1 - als1_pre2;
	} else {
		als1_diff2_abs = als1_pre2 - als1;
	}

	if ((als1 > RATIO_STOP_ALS2) && (als1 < 60000) && //210409 (als2 > RATIO_STOP_ALS2)
		((als1 / (als1_diff_abs + 1)) > RATIO_STOP_MULT) && ((als1 / (als1_diff2_abs + 1)) > RATIO_STOP_MULT) &&
		(als1_diff_abs > 0) && (als1_diff2_abs > 0)) {

		als_ir_ratio_128x = 128 * als2_pre * 1000 / als1_pre; // cwf_base_ratio is base on 1000 times
		adj_update_flag = 1;
	} else {
		// add new 
		als_adj = als_adj_pre;
	}
	als_ir_ratio = als_ir_ratio_128x / 128;

	if (new_sw1 == 1) {
		if (adj_update_flag == 1) {
			if (als1 < agc_thh) {
				if (als2 == 65535) {
					als_adj = als_adj_set2;
				} else {
					if (als_ir_ratio <= cwf_base_ratio_thl) {
						als_adj = 1000;
					} else if (als_ir_ratio > cwf_base_ratio_thl && als_ir_ratio <= cwf_base_ratio_thh) {
						als_adj = 1000 - ((1000 - als_adj_set)*(als_ir_ratio - cwf_base_ratio_thl) / (cwf_base_ratio_thh - cwf_base_ratio_thl));
					} else if (als_ir_ratio > cwf_base_ratio_thh && als_ir_ratio <= cwf_base_ratio_thh_d) {
						als_adj = als_adj_set;
					} else if (als_ir_ratio > cwf_base_ratio_thh_d && als_ir_ratio <= cwf_base_ratio_thh_a) {
						als_adj = als_adj_set2 + ((als_adj_set2 - als_adj_set)*(als_ir_ratio - cwf_base_ratio_thh) / (cwf_base_ratio_thh_a - cwf_base_ratio_thh));
					} else if (als_ir_ratio > cwf_base_ratio_thh_a) {
						als_adj = als_adj_set2;
					}
				}
			} else {
				als_adj = als_adj_pre;
			}
		} else {
			als_adj = als_adj_pre;
		}
		// adj filter
		als_adj = als_adj_pre * (RATIO_FLT_ALPHA - 1) / RATIO_FLT_ALPHA + als_adj / RATIO_FLT_ALPHA;
		als_normalized = als1 * als_adj / 1000;
		als_adj_pre = als_adj;

	} else {
		als_normalized = als1;
	}

	for (ii = agc_step_max; ii > agc_step; ii--) {
		if (ii == 7) {
			als_normalized = als_normalized * 2;
		} else {
			als_normalized = als_normalized * 4;
		}
	}

	als_normalized = als_normalized_pre * (RATIO_FLT_ALPHA - 2) / RATIO_FLT_ALPHA + (als_normalized * 2 / RATIO_FLT_ALPHA);

	als_normalized_pre = als_normalized;

	// lux = als_sen_mlux*als_normalized/100000;

	// if (als_normalized > 8000000) {
	// 	lux_10000x = (als_normalized / 100) * 477;
	// }
	// else {
	// 	lux_10000x = (477 * als_normalized) / 100;
	// }

	if (agc_lock_flag == 0) {
		hx3205_agc(i2c_client,als1, als2);
	}

#ifdef DEBUG
	dev_err(&i2c_client->dev,"als1=%d, als2=%d, als3=%d, agc=%d, ratio=%d, als_norm=%d, lux=%d, lux:%d,als_adj=%d, adj_update_flag=%d,\r\n", \
		als1, als2, als3, agc_step, als_ir_ratio, als_normalized, lux, lux_10000x,als_adj, adj_update_flag);
#endif

	return als_normalized;
}

static int hx3205_read(struct i2c_client *i2c_client)
{
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);
	int  val;

	mutex_lock(&data->mutex);
	val = i2c_smbus_read_byte_data(i2c_client, HX3205_ALS_ENABLE_REG);
	if(val < 0 || (val & 0x80) == 0) {
		printk("HX3205 StandBy\n");
		val = -1;
		mutex_unlock(&data->mutex);
		goto ret;
	}

	val = read_hx3205_als(i2c_client);
	mutex_unlock(&data->mutex);

ret:
	return val;
}

static irqreturn_t hx3205_gpio_irq_thread(int irq, void *dev_id)
{
	struct i2c_client *i2c_client= (struct i2c_client *)dev_id;
    struct hx3205_private *data = i2c_get_clientdata(i2c_client);

	#ifdef DEBUG
	dev_err(&i2c_client->dev, "Interrupt is triggered\n");
	#endif

	mutex_lock(&data->mutex);
	
	wake_up_interruptible(&data->r_wait);

    mutex_unlock(&data->mutex);
	return IRQ_HANDLED;
}

static int hx3205_register_interrupt(struct i2c_client *i2c_client)
{
	int ret = 0;
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);

	if (devm_gpio_request(&i2c_client->dev, data->int_gpio_num, "ALS INT") == 0) {
		data->int_gpio_irq_num  = gpio_to_irq(data->int_gpio_num);
		if (data->int_gpio_irq_num &&
		    devm_request_threaded_irq(&i2c_client->dev,data->int_gpio_irq_num, NULL,
								 hx3205_gpio_irq_thread,
								 IRQF_TRIGGER_RISING | IRQF_ONESHOT,
								 dev_name(&i2c_client->dev), i2c_client) == 0) {
			dev_info(&i2c_client->dev, "request irq succeed\n");
		} else {
			dev_err(&i2c_client->dev, "request irq failed\n");
			data->int_gpio_irq_num = 0;
			ret = -1;
		}
	} else {
		dev_err(&i2c_client->dev, "request gpio failed\n");
		ret = -2;
	}

	return ret;
}

static ssize_t hx3205_enable_store(struct device *dev,
		struct device_attribute *attr, const  char *buf, size_t count)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);
	int ret_val;
	unsigned long val;

	ret_val = kstrtoul(buf, 10, &val);
	if (ret_val) {
		return ret_val;
	}

	mutex_lock(&data->mutex);

	ret_val = i2c_smbus_read_byte_data(i2c_client, HX3205_ALS_ENABLE_REG);
	if (ret_val < 0) {
		goto fail;
	}

	ret_val = ret_val & 0x7F;

	if (val != 0) {
		ret_val = (ret_val | 0x80);	
	}

	ret_val = i2c_smbus_write_byte_data(i2c_client, HX3205_ALS_ENABLE_REG, ret_val);

	if (ret_val >= 0) {
		mutex_unlock(&data->mutex);
		return count;
	}

fail:
	mutex_unlock(&data->mutex);

	return ret_val;
}

static ssize_t hx3205_status_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);
	int  val;

	mutex_lock(&data->mutex);
	val = i2c_smbus_read_byte_data(i2c_client, HX3205_ALS_ENABLE_REG);
	mutex_unlock(&data->mutex);
	#ifdef DEBUG
	dev_err(dev,"HX3205_ALS_ENABLE_REG:0x%x\n",val);
	#endif
	if (val < 0) {
		return val;
	}
	if (val & 0x80) {
		return sprintf(buf, "Active\n");
	} else {
		return sprintf(buf, "Stand-by\n");
	}	
}

static ssize_t hx3205_value_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);

	return sprintf(buf, "%d\n", hx3205_read(i2c_client));
}

static ssize_t hx3205_interrupt_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);
	int timeout = data->timeout;
	int ret = 0;
	DECLARE_WAITQUEUE(wait, current);

	#ifdef DEBUG
	dev_err(dev,"timeout:%d\n",timeout);
	#endif

	add_wait_queue(&data->r_wait, &wait);
	__set_current_state(TASK_INTERRUPTIBLE);
	if(timeout == 0) {
		schedule();
	} else {
		ret = schedule_timeout(HZ * timeout);
		if(!ret){
			goto wait_error;
		}
	}
	if(signal_pending(current)) {
		goto wait_error;
	}
	__set_current_state(TASK_RUNNING);
	remove_wait_queue(&data->r_wait, &wait);

	return sprintf(buf, "%d\n", hx3205_read(i2c_client));

wait_error:
	set_current_state(TASK_RUNNING); /* 设置任务为运行态 */
	remove_wait_queue(&data->r_wait, &wait); /* 将等待队列移除 */
	return sprintf(buf, "-99\n");
}

static ssize_t hx3205_interrupt_store(struct device *dev,
		struct device_attribute *attr, const  char *buf, size_t count)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);
	unsigned long val;
	int ret = 0;

	ret = kstrtoul(buf, 10, &val);
	if (ret < 0) {
		return 0;
	}
	
	mutex_lock(&data->mutex);
	data->timeout = val;
	mutex_unlock(&data->mutex);
	
	#ifdef DEBUG
	dev_err(dev,"timeout:%d\n",val);
	#endif

	return count;
}

static ssize_t hx3205_upper_threshold_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);
	unsigned int  val,val1, val2;

	mutex_lock(&data->mutex);
	val1 = i2c_smbus_read_byte_data(i2c_client, HX3205_UPPER_THRESHOLD_L_REG);
	val2 = i2c_smbus_read_byte_data(i2c_client, HX3205_UPPER_THRESHOLD_H_REG);
	val = val1 | (val2 << 8);
	mutex_unlock(&data->mutex);
	#ifdef DEBUG
	dev_err(dev,"HX3205_UPPER_THRESHOLD_H_REG:0x%x.%d\n",val,val);
	dev_err(dev,"HX3205_UPPER_THRESHOLD_H_REG0:0x%x HX3205_UPPER_THRESHOLD_H_REG1:0x%x HX3205_UPPER_THRESHOLD_H_REG:0x%x\n",val1,val2,val1 | (val2 << 8));
	#endif

	return sprintf(buf, "%d\n", val);
}

static ssize_t hx3205_upper_threshold_store(struct device *dev,
		struct device_attribute *attr, const  char *buf, size_t count)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);
	int ret_val;
	unsigned char data0,data1;
	unsigned long val;

	ret_val = kstrtoul(buf, 10, &val);
	if (ret_val && val > data->lower_threshold && val != data->upper_threshold) {
		return ret_val;
	}

	data->upper_threshold = val;
	mutex_lock(&data->mutex);

	data1 = ((val & 0xff00) >> 8);
    data0 = (val & 0x00ff);

	ret_val = i2c_smbus_write_byte_data(i2c_client, HX3205_UPPER_THRESHOLD_L_REG, data0);
    ret_val = i2c_smbus_write_byte_data(i2c_client, HX3205_UPPER_THRESHOLD_H_REG, data1);
	if (ret_val >= 0) {
		mutex_unlock(&data->mutex);
		return count;
	}

	mutex_unlock(&data->mutex);

	return ret_val;
}

static ssize_t hx3205_lower_threshold_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);
	unsigned int  val,val1, val2;

	mutex_lock(&data->mutex);
	val1 = i2c_smbus_read_byte_data(i2c_client, HX3205_LOWER_THRESHOLD_L_REG);
	val2 = i2c_smbus_read_byte_data(i2c_client, HX3205_LOWER_THRESHOLD_H_REG);
	val = val1 | (val2 << 8);
	mutex_unlock(&data->mutex);
	#ifdef DEBUG
	dev_err(dev,"HX3205_LOWER_THRESHOLD_L_REG:0x%x.%d\n",val,val);
	dev_err(dev,"HX3205_LOWER_THRESHOLD_L_REG0:0x%x HX3205_LOWER_THRESHOLD_L_REG1:0x%x HX3205_LOWER_THRESHOLD_L_REG:0x%x\n",val1,val2,val1 | (val2 << 8));
	#endif
	if (val < 0) {
		return val;
	}

	return sprintf(buf, "%d\n", val);
}

static ssize_t hx3205_lower_threshold_store(struct device *dev,
		struct device_attribute *attr, const  char *buf, size_t count)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct hx3205_private *data = i2c_get_clientdata(i2c_client);
	int ret_val;
	unsigned char data0,data1;
	unsigned long val;

	ret_val = kstrtoul(buf, 10, &val);
	if (ret_val && val < data->upper_threshold && val != data->lower_threshold) {
		return ret_val;
	}

	data->lower_threshold = val;
	mutex_lock(&data->mutex);

	data1 = ((val & 0xff00) >> 8);
    data0 = (val & 0x00ff);

	ret_val = i2c_smbus_write_byte_data(i2c_client, HX3205_LOWER_THRESHOLD_L_REG, data0);
    ret_val = i2c_smbus_write_byte_data(i2c_client, HX3205_LOWER_THRESHOLD_H_REG, data1);
	if (ret_val >= 0) {
		mutex_unlock(&data->mutex);
		return count;
	}

	mutex_unlock(&data->mutex);

	return ret_val;
}

static DEVICE_ATTR(enable, S_IWUSR, NULL, hx3205_enable_store);
static DEVICE_ATTR(status, S_IRUGO, hx3205_status_show, NULL);
static DEVICE_ATTR(value, S_IRUGO, hx3205_value_show, NULL);
static DEVICE_ATTR(interrupt, S_IRUGO | S_IWUSR, hx3205_interrupt_show, hx3205_interrupt_store);
static DEVICE_ATTR(upper_threshold, S_IRUGO | S_IWUSR, hx3205_upper_threshold_show, hx3205_upper_threshold_store);
static DEVICE_ATTR(lower_threshold, S_IRUGO | S_IWUSR, hx3205_lower_threshold_show, hx3205_lower_threshold_store);

static struct attribute *hx3205_mid_att[] = {
	&dev_attr_enable.attr,
	&dev_attr_status.attr,
	&dev_attr_value.attr,
	&dev_attr_interrupt.attr,
	&dev_attr_upper_threshold.attr,
	&dev_attr_lower_threshold.attr,
	NULL
};

static struct attribute_group hx3205_gr = {
	.name = "property",
	.attrs = hx3205_mid_att
};

static int hx3205_i2c_probe(struct i2c_client *i2c_client,
					const struct i2c_device_id *id)
{
	int ret,try = 3;

	dev_info(&i2c_client->dev, "hx3205_i2c_probe\n");
	
	do
	{
		ret = i2c_smbus_read_byte_data(i2c_client, HX3205_MANUFAC_ID_REG);
		if (ret < 0) {
			printk("read hx3205 Manufac ID failed.ret:%d\n",ret);		
			try--;
		}
	} while (ret < 0 && try > 0);

	if (ret < 0) {
		return ret;
	}
	
	if(ret != 0x25) {
		dev_err(&i2c_client->dev,"hx3205 Manufac ID:0x%x mismatching\n",ret);	
		return -ENXIO;
	}

	dev_info(&i2c_client->dev,"hx3205 Manufac ID:0x%x\n",ret);

	hx3205_data = devm_kzalloc(&i2c_client->dev,sizeof(struct hx3205_private), GFP_KERNEL);
	if(hx3205_data == NULL) {
		dev_err(&i2c_client->dev, "devm_kzalloc failed\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(i2c_client, hx3205_data);
	mutex_init(&hx3205_data->mutex);
	hx3205_data->i2c_client = i2c_client;

	ret = hx3205_init(i2c_client);
	if(ret) {
		dev_err(&i2c_client->dev, "hx3205_init failed:%d\n", ret);
		goto error;
	}

	init_waitqueue_head(&hx3205_data->r_wait);
	hx3205_data->timeout = 0;
	hx3205_data->int_gpio_num = of_get_named_gpio_flags(i2c_client->dev.of_node, "int-gpios", 0, NULL);
	if(gpio_is_valid(hx3205_data->int_gpio_num)) {
		hx3205_register_interrupt(i2c_client);
	} else {
		dev_err(&i2c_client->dev, "The interrupt GPIO is invalid\n");
	}

    als_kset = kset_create_and_add("als", NULL, NULL);
    
	if (!als_kset) {
		printk(KERN_WARNING "%s (%d): error creating kset\n",
			__FILE__, __LINE__);
		return -ENOMEM;
	}

	ret = sysfs_create_group(&i2c_client->dev.kobj, &hx3205_gr);
	if (ret) {
		dev_err(&i2c_client->dev, "device create file failed:%d\n", ret);
	}

    sysfs_create_link(&als_kset->kobj,&i2c_client->dev.kobj,"device");

error:
	return ret;
}

static  int hx3205_i2c_remove(struct i2c_client *i2c_client)
{
	dev_info(&i2c_client->dev, "hx3205_i2c_remove\n");

	return 0;
}

static void hx3205_i2c_shutdown(struct i2c_client *i2c_client)
{
	dev_info(&i2c_client->dev, "hx3205_i2c_shutdown\n");

	return;
}

static struct i2c_device_id hx3205_id[] = {
	{ "hx3205", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, hx3205_id);

static struct i2c_driver hx3205_driver = {
	.driver = {
		.name = "hx3205",
	},
	.probe = hx3205_i2c_probe,
	.remove = hx3205_i2c_remove,
	.shutdown = hx3205_i2c_shutdown,
	.id_table = hx3205_id,
};

static int __init hx3205_module_init(void)
{
	int ret;

	printk("hx3205_module_init\n");

	ret = i2c_add_driver(&hx3205_driver);
	if (ret < 0) {
		printk("Unable to register hx3205 driver, ret= %d", ret);
		return ret;
	}

	return 0;
}

static void __exit hx3205_module_exit(void)
{
	printk("hx3205_module_exit\n");
	i2c_del_driver(&hx3205_driver);
}

module_init(hx3205_module_init);
module_exit(hx3205_module_exit);

MODULE_AUTHOR("wangchuanqi <wangchuanqi@70mai.com");
MODULE_DESCRIPTION("hx3205 ALS Driver");
MODULE_LICENSE("GPL v2");