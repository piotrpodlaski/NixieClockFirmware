#include "driver/spi_master.h"

template<uint8_t numRegs>
class ShiftRegTPIC {
  public:
    ShiftRegTPIC(int mosi, int clk, int latch, int g, int clr, int spi_freq_hz = 1000000)
      : ioMOSI{mosi}, ioCLK{clk}, ioLATCH{latch}, ioG{g}, ioCLR{clr}, spi_freq{spi_freq_hz} {
      initSPI();
      initPins();
      clearRegisters();
    }

    void clearRegisters() {
      digitalWrite(ioCLR, LOW);
      digitalWrite(ioCLR, HIGH);
      latch();
    }

    void latch() {
      digitalWrite(ioLATCH, HIGH);
      digitalWrite(ioLATCH, LOW);
    }

    void zeroData() {
      memset(digitalValues, 0, numRegs * sizeof(uint8_t));
    }

    void setSingle(uint8_t chip, uint8_t pin,bool val){
      if(chip>=numRegs) return;
      if(pin>=8) return;
      val ? bitSet(digitalValues[chip], pin) : bitClear(digitalValues[chip], pin);
    }

    void setSingle(uint8_t globalPin ,bool val){
      setSingle(globalPin/8, globalPin % 8);
    }


    void updateRegisters() {
      spi_transaction_t t;
      memset(&t, 0, sizeof(t));
      t.length = numRegs*8; //in bits;
      t.tx_buffer = digitalValues;
      t.user = (void*)1;
      auto ret = spi_device_polling_transmit(spi, &t); //Transmit!
      ESP_ERROR_CHECK(ret);
      latch();
    }
  private:

    void initSPI() {
      spi_bus_config_t buscfg;
      memset(&buscfg, 0, sizeof(spi_bus_config_t));
      buscfg.miso_io_num = -1;
      buscfg.mosi_io_num = 23;
      buscfg.sclk_io_num = 18;
      buscfg.quadwp_io_num = -1;
      buscfg.quadhd_io_num = -1;

      spi_device_interface_config_t devcfg;
      memset(&devcfg, 0, sizeof(spi_device_interface_config_t));
      devcfg.clock_speed_hz = spi_freq;
      devcfg.mode = 0;
      devcfg.spics_io_num = -1;
      devcfg.queue_size = 1;

      auto ret = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
      ESP_ERROR_CHECK(ret);

      //attach the device to the bus
      ret = spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
      ESP_ERROR_CHECK(ret);
    }

    void initPins() {
      pinMode(ioLATCH, OUTPUT);
      pinMode(ioG, OUTPUT);
      pinMode(ioCLR, OUTPUT);

      digitalWrite(ioLATCH, LOW);
      digitalWrite(ioG, HIGH); //disable out
      digitalWrite(ioCLR, HIGH);
    }

    int ioMOSI, ioCLK, ioLATCH, ioG, ioCLR, spi_freq;
    spi_device_handle_t spi;
    uint8_t digitalValues[numRegs];
};
