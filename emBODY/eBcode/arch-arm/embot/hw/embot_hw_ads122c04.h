
/*
 * Copyright (C) 2020 iCub Facility - Istituto Italiano di Tecnologia
 * Author:  Andrea Mura
 * email:   andrea.mura@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

// - include guard ----------------------------------------------------------------------------------------------------

#ifndef _EMBOT_HW_ADS122C04_H_
#define _EMBOT_HW_ADS122C04_H_

#include "embot_core.h"
#include "embot_core_utils.h"
#include "embot_core_binary.h"
#include "embot_hw.h"


#include "embot_hw_i2c.h"

    
namespace embot { namespace hw { namespace ads122c04 {
                 
    struct Config
    {  
        embot::hw::i2c::Descriptor      i2cdes;        
        Config(embot::hw::I2C b, std::uint32_t s) : i2cdes(b, s) {}        
        Config() : i2cdes(embot::hw::I2C::one, 400000) {}
        Config(const embot::hw::i2c::Descriptor &des) : i2cdes(des) {}
    };
    
    
    enum class Mode 
    { 
        // Page 33 of datasheet
				SingleShotConversionMode = 0, 
        ContinuousConversionMode = 1,
				NormalMode = 2,
				TurboMode	= 3,
				PowerDownMode = 4
    };
        
     
    enum class Register 
    { 
        CHIP_ID         = 0x00, // 7 bytes RO with many info. see embot::hw::ads122c04::Info 
        PAGE_ID         = 0x07, // 1 byte RW. it changes page. unfortunately there are no registers defined for page 1
        DATASET_START   = 0x08, // how many as we want: acc + mag + gyr + eul + qua + lia + grv + temp + calibstat + stresult (total = 47 bytes).
        //PWR_MODE        = 0x3E
    };  

    enum class Set
    {
        A               = 6,        //     
        FULL            = 47        // total bytes     
    };
    
          
    struct Info
    {   // 7 registers from Register::CHIP_ID
        std::uint8_t    chipID;         // RO: 0xA0     
        Info() { clear(); }
        void clear() { chipID = 0; }
        void load(void *mem) 
        { 
            std::uint8_t *m = reinterpret_cast<std::uint8_t*>(mem); 
            chipID = m[0];  
        }
        bool isvalid() { if((0xA0==chipID)){return true;} return false; }
    };
         
    static const float accRES = 0.01f;              // = 1/100 [m/(s*s)]
    static const float tmpRES = 1.0f;              // = 1 [Celsius Deg]
    
    struct Data
    {   // it holds the Set::FULL lot
        embot::core::utils::Triple<std::int16_t>     acc;    // acc.z = 9.8 means: horizontally placed 
        embot::core::utils::Triple<std::int16_t>     mag;
        embot::core::utils::Triple<std::int16_t>     gyr;
        embot::core::utils::Triple<std::int16_t>     eul;    // eul.x = 0 means: directed towards NORTH
        embot::core::utils::Quadruple<std::int16_t>  qua;
        embot::core::utils::Triple<std::int16_t>     lia;
        embot::core::utils::Triple<std::int16_t>     grv;
        std::uint8_t                            temperature;
        std::uint8_t                            calibstatus;
        std::uint8_t                            systemstatus;  
        Data() { clear(); }
        void clear() 
        {
            acc.clear();
            mag.clear();
            gyr.clear();
            eul.clear();
            qua.clear();
            lia.clear();
            grv.clear();
            temperature = 0;
            calibstatus = 0;
            systemstatus = 0;
        }
        void load(void *mem) 
        {
            std::uint8_t *m = reinterpret_cast<std::uint8_t*>(mem); 
            acc.load(m); mag.load(&m[6]); gyr.load(&m[12]); eul.load(&m[18]);
            qua.load(&m[24]);
            lia.load(&m[32]); grv.load(&m[38]);
            temperature = m[44]; calibstatus = m[45]; systemstatus = m[46];
        }
        void getACC(embot::core::utils::Triple<float> &a) const { a.x = accRES * acc.x; a.y = accRES * acc.y; a.z = accRES * acc.z; }
        void getTemperature(float &a) const { a = tmpRES * temperature; }
        std::uint8_t calibrationOfACC() const { return(embot::core::binary::pair::get(calibstatus, 1)); } // 3 is ok, 1 is not calibrated
        std::uint8_t calibrationOfGYR() const { return(embot::core::binary::pair::get(calibstatus, 2)); } // 3 is ok, 1 is not calibrated
        std::uint8_t calibrationOfMAG() const { return(embot::core::binary::pair::get(calibstatus, 0)); } // 3 is ok, 1 is not calibrated
        //std::uint8_t calibrationOfSYS() const { return(embot::core::binary::pair::get(calibstatus, 4)); } // 3 is ok, 1 is not calibrated BUT ALWAYS ZERO
    };        
    
    
    
    bool supported(ADS122C04 s);
    
    bool initialised(ADS122C04 s);
    
    // the init() starts the chip, prepares i2c, pings it. if all ok it returns resOK.
    // the working mode is however Mode::CONFIG
    result_t init(ADS122C04 s, const Config &config);
    
    // after that init() returns resOK we can check if it is alive. we can specify a timeout
    bool isalive(ADS122C04 s, embot::core::relTime timeout = 3*embot::core::time1millisec);
    
    // we can get info
    result_t get(ADS122C04 s, Info &info, embot::core::relTime timeout = 3*embot::core::time1millisec);
    
    // we can now set a working mode. we can specify a timeout
    result_t set(ADS122C04 s, Mode m, embot::core::relTime timeout = 3*embot::core::time1millisec);
    
    // we must check that nobody is using the sensor, maybe in non-blocking mode some time earlier
    bool isacquiring(ADS122C04 s);
    
    // we check isacquiring() but also if any other device is using i2c bus
    bool canacquire(ADS122C04 s);
    
    // we start acquisition of a set.
    // if returns resOK, we know that acquisition is over if it is called oncompletion() or when operationdone() is true;
    result_t acquisition(ADS122C04 s, Set set, const embot::core::Callback &oncompletion = embot::core::Callback(nullptr, nullptr));
    
    result_t acquisition(ADS122C04 s, Set set, Data &data, const embot::core::Callback &oncompletion = embot::core::Callback(nullptr, nullptr));
    
    // it tells if a previous operation (acquisition, read, write) is over
    bool operationdone(ADS122C04 s);
    
    // ok, now we can read data previously acquired
    result_t read(ADS122C04 s, Data &data);
    
    // and here is acquisition in blocking mode
    result_t acquisition(ADS122C04 s, Set set, Data &data, const embot::core::relTime timeout);
    
    // here are some write() and read() funtions which operate directly on a single register reg 
    
    
    // we write a single byte into a register.
    // blocking or non-blocking mode. we check end of oepration either with operationdone() or with the execution of oncompletion().      
    result_t write(ADS122C04 s, embot::hw::ads122c04::Register reg, std::uint8_t value, const embot::core::relTime timeout);
    result_t write(ADS122C04 s, embot::hw::ads122c04::Register reg, std::uint8_t value, const embot::core::Callback &oncompletion = embot::core::Callback(nullptr, nullptr));

    // we read from register reg, for data.size positions and we place results into data.pointer (which MUST point to at least data.size bytes)
    // blocking or non-blocking mode. we check end of oepration either with operationdone() or with the execution of oncompletion().  
    result_t read(ADS122C04 s, embot::hw::ads122c04::Register reg, embot::core::Data &data, const embot::core::relTime timeout);
    result_t read(ADS122C04 s, embot::hw::ads122c04::Register reg, embot::core::Data &data, const embot::core::Callback &oncompletion = embot::core::Callback(nullptr, nullptr));

 
}}} // namespace embot { namespace hw { namespace ads122c04 {
    


#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------


