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


// --------------------------------------------------------------------------------------------------------------------
// - public interface
// --------------------------------------------------------------------------------------------------------------------

#include "embot_hw_ads122c04.h"



// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include <cstring>
#include <vector>
#include "stm32hal.h"
#include "embot_core_binary.h"
#include "embot_hw_sys.h"
#include "embot_hw_gpio.h"
#include "embot_hw_bsp.h"
#include "embot_hw_bsp_config.h"

using namespace std;

using namespace embot::hw;

// --------------------------------------------------------------------------------------------------------------------
// - pimpl: private implementation (see scott meyers: item 22 of effective modern c++, item 31 of effective c++
// --------------------------------------------------------------------------------------------------------------------

#warning FIX-it: EMBOT_ENABLE_hw_ads122c04

#if !defined(EMBOT_ENABLE_hw_ads122c04)
    #define EMBOT_ENABLE_hw_ads122c04
#endif

// --------------------------------------------------------------------------------------------------------------------
// - all the rest
// --------------------------------------------------------------------------------------------------------------------

#if !defined(EMBOT_ENABLE_hw_ads122c04)

namespace embot { namespace hw { namespace ads122c04 {

    bool supported(ADS122C04 s)                                                                        { return false; }
    bool initialised(ADS122C04 s)                                                                      { return false; }
    result_t init(ADS122C04 s, const Config &config)                                                   { return resNOK; }
    bool isalive(ADS122C04 s, embot::core::relTime timeout)                                          { return false; }
    result_t get(ADS122C04 s, Info &info, embot::core::relTime timeout)                              { return resNOK; }
    result_t set(ADS122C04 s, Mode m, embot::core::relTime timeout)                                  { return resNOK; }
    bool isacquiring(ADS122C04 s)                                                                      { return false; }
    bool canacquire(ADS122C04 s)                                                                       { return false; }
    result_t acquisition(ADS122C04 s, Set set, const embot::core::Callback &oncompletion)            { return resNOK; }
    bool operationdone(ADS122C04 s)                                                                    { return false; }
    result_t read(ADS122C04 s, Data &data)                                                             { return resNOK; }
    result_t write(ADS122C04 s, embot::hw::ads122c04::Register reg, std::uint8_t value, const embot::core::relTime timeout){ return resNOK; }
    result_t write(ADS122C04 s, embot::hw::ads122c04::Register reg, std::uint8_t value, const embot::core::Callback &oncompletion){ return resNOK; }
    result_t read(ADS122C04 s, embot::hw::ads122c04::Register reg, embot::core::Data &data, const embot::core::relTime timeout){ return resNOK; }
    result_t read(ADS122C04 s, embot::hw::ads122c04::Register reg, embot::core::Data &data, const embot::core::Callback &oncompletion){ return resNOK; }
    
}}} // namespace embot { namespace hw { namespace ADS122C04 {


#else

namespace embot { namespace hw { namespace ads122c04 {
        
    // initialised mask       
    static std::uint32_t initialisedmask = 0;
    
    bool supported(ADS122C04 s)
    {
        #warning FIX-it: BSP::supported()
        //return embot::hw::bsp::ads122c04::getBSP().supported(s);
        return true;
    }
    
    bool initialised(ADS122C04 s)
    {
        return embot::core::binary::bit::check(initialisedmask, embot::core::tointegral(s));
    }     

    struct Acquisition
    {
        volatile bool done;
        volatile bool ongoing;
        std::uint8_t buffer[48];
        embot::core::Data data;
        embot::core::Callback userdefCBK;  
        Acquisition() { clear(); }
        void clear() { data.load(buffer, 0); done = false; ongoing = false; std::memset(buffer, 0, sizeof(buffer)); userdefCBK.clear(); }         
        void startread(std::uint8_t size, const embot::core::Callback &cbk) { clear(); data.capacity = size; userdefCBK = cbk; ongoing = true; }
        void startread(std::uint8_t size) { clear(); data.capacity = size; ongoing = true; }
        void startread(embot::core::Data &da, const embot::core::Callback &cbk) { clear(); data = da; userdefCBK = cbk; ongoing = true; }
        void startread(embot::core::Data &da) { clear(); data = da; ongoing = true; }
        void startwrite(std::uint8_t val) { clear(); buffer[0] = val; data.capacity = 1; ongoing = true; }
        void startwrite(std::uint8_t val, const embot::core::Callback &cbk) { clear(); buffer[0] = val; data.capacity = 1; userdefCBK = cbk; ongoing = true; }
        void stop() { ongoing = false; done = true; userdefCBK.execute(); }
    };
    
    struct PrivateData
    {    
        std::uint8_t i2caddress[embot::core::tointegral(ADS122C04::maxnumberof)];
        Config config[embot::core::tointegral(ADS122C04::maxnumberof)];        
        Acquisition acquisition[embot::core::tointegral(ADS122C04::maxnumberof)];
        PrivateData() { }
    };
    
    // power-on-reset wait time. 650 ms is enough, datasheet say, before the device can talk over i2c. but much better to be devil-compliant ;-).
    static const embot::core::relTime PORtime = 666*embot::core::time1millisec;
    
//    static const std::uint8_t i2caddress = 0x52;
//    #if     defined(STM32HAL_BOARD_STRAIN2)
//        static const embot::hw::GPIO gpioBOOT(ADS122C04_BOOT_GPIO_Port, ADS122C04_BOOT_Pin);
//        static const embot::hw::GPIO gpioRESET(ADS122C04_RESET_GPIO_Port, ADS122C04_RESET_Pin);
//    #elif   defined(STM32HAL_BOARD_MTB4)
//        static const embot::hw::GPIO gpioBOOT(ADS122C04_BOOT_GPIO_Port, ADS122C04_BOOT_Pin);
//        static const embot::hw::GPIO gpioRESET(ADS122C04_RESET_GPIO_Port, ADS122C04_RESET_Pin);  
//    #elif   defined(STM32HAL_BOARD_RFE)
//        static const embot::hw:::GPIO gpioBOOT(ADS122C04_BOOT_GPIO_Port, ADS122C04_BOOT_Pin);
//        static const embot::hw:::GPIO gpioRESET(ADS122C04_RESET_GPIO_Port, ADS122C04_RESET_Pin);     
//    #endif
    
    static PrivateData s_privatedata;
     
    static void s_powerON(ADS122C04 s, embot::core::relTime waittime);    
    static result_t s_programregister(ADS122C04 s, std::uint8_t reg, std::uint8_t val, embot::core::relTime timeout);    
    static result_t s_writeregister(ADS122C04 s, std::uint8_t reg, std::uint8_t val, const embot::core::Callback &oncompletion);
    static void s_sharedCBK(void *p);
    
            
    result_t init(ADS122C04 s, const Config &config)
    {
        if(false == supported(s))
        {
            return resNOK;
        }
        
        if(true == initialised(s))
        {
            return resOK;
        }
        
        #warning FIX-it: BSP::init()
        // init peripheral
        //embot::hw::bsp::ads122c04::getBSP().init(s);
        
        // power on and wait until the i2c is working. datasheet say: POR time > 650 ms.
        s_powerON(s, PORtime);
        
        std::uint8_t index = embot::core::tointegral(s);
        
                        
        // init i2c ..
        embot::hw::i2c::init(config.i2cdes.bus, config.i2cdes.config);
        
#warning FIX-it: i2c address
//        embot::hw::i2c::ADR i2caddress = embot::hw::bsp::ads122c04::getBSP().getPROP(s)->i2caddress;
        embot::hw::i2c::ADR i2caddress = 1;
        if(false == embot::hw::i2c::ping(config.i2cdes.bus, i2caddress, 3*embot::core::time1millisec))
        {
            return resNOK;
        }
        
        s_privatedata.config[index] = config;
        s_privatedata.acquisition[index].clear();
        s_privatedata.i2caddress[index] = i2caddress;
        
        embot::core::binary::bit::set(initialisedmask, embot::core::tointegral(s));
                
        return resOK;
    }



    bool isalive(ADS122C04 s, embot::core::relTime timeout)
    {
        if(false == initialised(s))
        {
            return false;
        } 
        std::uint8_t index = embot::core::tointegral(s);
        return embot::hw::i2c::ping(s_privatedata.config[index].i2cdes.bus, s_privatedata.i2caddress[index], timeout);  
    }        


    
    result_t get(ADS122C04 s, Info &info, embot::core::relTime timeout)
    {
        embot::core::Data data(&info, sizeof(info));
        result_t r = read(s, embot::hw::ads122c04::Register::CHIP_ID, data, timeout);  
        return r;        
    }
    
    
    result_t set(ADS122C04 s, Mode m, embot::core::relTime timeout)
    { 
        if(false == initialised(s))
        {
            return resNOK;
        }  
        
        if((Mode::none == m) || (Mode::maxnumberof == m))
        {
            return resNOK;
        }
               
        return write(s, Register::OPR_MODE, static_cast<std::uint8_t>(m), timeout);                                       
    } 


    bool isacquiring(ADS122C04 s)
    {
        if(false == initialised(s))
        {
            return false;
        } 

        std::uint8_t index = embot::core::tointegral(s);        
        return s_privatedata.acquisition[index].ongoing;     
    }    
    
        
    bool canacquire(ADS122C04 s)
    {
        if(false == initialised(s))
        {
            return false;
        } 

        std::uint8_t index = embot::core::tointegral(s);  
        
        if(true == s_privatedata.acquisition[index].ongoing)
        {
            return false;
        }
        
        return !embot::hw::i2c::isbusy(s_privatedata.config[index].i2cdes.bus);             
    }
    
    
    
    result_t acquisition(ADS122C04 s, Set set, const embot::core::Callback &oncompletion)
    {        
        if(false == canacquire(s))
        {
            return resNOK;
        }
        
        // ok, start a read of nbytes which will go into buffer[]
        const std::uint8_t nbytes = static_cast<std::uint8_t>(set);
        const Register reg = Register::DATASET_START;
        
        std::uint8_t index = embot::core::tointegral(s);
        
        s_privatedata.acquisition[index].startread(nbytes, oncompletion); 
                       
        // now i trigger i2c.
        embot::core::Callback cbk(s_sharedCBK, &s_privatedata.acquisition[index]);
        return embot::hw::i2c::read(s_privatedata.config[index].i2cdes.bus, s_privatedata.i2caddress[index], static_cast<std::uint8_t>(reg), s_privatedata.acquisition[index].data, cbk);        
    } 
    
    
    
    result_t acquisition(ADS122C04 s, Set set, Data &data, const embot::core::Callback &oncompletion)
    {        
        if(false == canacquire(s))
        {
            return resNOK;
        }
        
        std::uint8_t index = embot::core::tointegral(s);
        
        const std::uint8_t nbytes = static_cast<std::uint8_t>(set);        
        // ok, start a read of nbytes only (not all sizeof(data)) which will go into data
        embot::core::Data da(&data, nbytes);
                
        s_privatedata.acquisition[index].startread(da, oncompletion); 
                       
        // now i trigger i2c.
        embot::core::Callback cbk(s_sharedCBK, &s_privatedata.acquisition[index]);
        return embot::hw::i2c::read(s_privatedata.config[index].i2cdes.bus, s_privatedata.i2caddress[index], static_cast<std::uint8_t>(Register::DATASET_START), s_privatedata.acquisition[index].data, cbk);        
    } 
    

    result_t acquisition(ADS122C04 s, Set set, Data &data, const embot::core::relTime timeout)
    { 
        const std::uint8_t nbytes = static_cast<std::uint8_t>(set);        
        // ok, start a read of nbytes only (not all sizeof(data)) which will go into data
        embot::core::Data da(&data, nbytes);
        
        return read(s, Register::DATASET_START, da, timeout);
    }     
    
    bool operationdone(ADS122C04 s)
    {
        if(false == initialised(s))
        {
            return false;
        } 

        return s_privatedata.acquisition[embot::core::tointegral(s)].done;        
    } 

    
    result_t read(ADS122C04 s, Data &data)
    { 
        if(false == initialised(s))
        {
            return resNOK;
        } 

        if(false == operationdone(s))
        {
            return resNOK;
        }
        
        std::uint8_t index = embot::core::tointegral(s);
        data.load(s_privatedata.acquisition[index].data.pointer);
        
        return resOK;   
    } 


    result_t write(ADS122C04 s, embot::hw::ads122c04::Register reg, std::uint8_t value, const embot::core::relTime timeout)
    {
        if(false == initialised(s))
        {
            return resNOK;
        } 
        
        std::uint8_t index = embot::core::tointegral(s);
        
        // i2c must not be used by another device (eg. termometer or ...).
        embot::core::relTime remaining = timeout;
        if(true == embot::hw::i2c::isbusy(s_privatedata.config[index].i2cdes.bus, timeout, remaining))
        {
            return resNOK;
        }
        
        // if (when) i2c is not busy (anymore), we just hope there is no another task using the ads122c04 ...
        // we however just do the check         
        if(true == s_privatedata.acquisition[index].ongoing)
        {
            return resNOK;
        }
        
        return s_programregister(s, static_cast<std::uint8_t>(reg), value, remaining);                         
    }

    
    result_t write(ADS122C04 s, embot::hw::ads122c04::Register reg, std::uint8_t value, const embot::core::Callback &oncompletion)
    {
        if(false == initialised(s))
        {
            return resNOK;
        } 

        std::uint8_t index = embot::core::tointegral(s);

        // i2c must not be busy
        if(true == embot::hw::i2c::isbusy(s_privatedata.config[index].i2cdes.bus))
        {
            return resNOK;
        } 
        
        if(true == s_privatedata.acquisition[index].ongoing)
        {
            return resNOK;
        }
        
        return s_writeregister(s, static_cast<std::uint8_t>(reg), value, oncompletion);                         
    }        
       

        
    result_t read(ADS122C04 s, embot::hw::ads122c04::Register reg, embot::core::Data &data, const embot::core::relTime timeout)
    {
        if(false == initialised(s))
        {
            return resNOK;
        } 

        std::uint8_t index = embot::core::tointegral(s);
        
        // i2c must not be used by another device (eg. termometer or ...).
        embot::core::relTime remaining = timeout;
        if(true == embot::hw::i2c::isbusy(s_privatedata.config[index].i2cdes.bus, timeout, remaining))
        {
            return resNOK;
        }
        
        // if (when) i2c is not busy (anymore), we just hope there is no another task using the ads122c04 ...
        // we however just do the check         
        if(true == s_privatedata.acquisition[index].ongoing)
        {
            return resNOK;
        }
                
        s_privatedata.acquisition[index].startread(data);   
        
        // ok, now i trigger i2c in blocking mode
        result_t r = embot::hw::i2c::read(s_privatedata.config[index].i2cdes.bus, s_privatedata.i2caddress[index], static_cast<std::uint8_t>(reg), s_privatedata.acquisition[index].data, timeout);
        s_privatedata.acquisition[index].stop();
        
        return r;      
    }

    
    result_t read(ADS122C04 s, embot::hw::ads122c04::Register reg, embot::core::Data &data, const embot::core::Callback &oncompletion)
    {
        if(false == initialised(s))
        {
            return resNOK;
        } 

        std::uint8_t index = embot::core::tointegral(s);
        
        // i2c must not be busy
        if(true == embot::hw::i2c::isbusy(s_privatedata.config[index].i2cdes.bus))
        {
            return resNOK;
        } 
        
        if(true == s_privatedata.acquisition[index].ongoing)
        {
            return resNOK;
        }
        
        s_privatedata.acquisition[index].startread(data, oncompletion); 
                       
        // ok, now i trigger i2c.
        embot::core::Callback cbk(s_sharedCBK, &s_privatedata.acquisition[index]);
        embot::hw::i2c::read(s_privatedata.config[index].i2cdes.bus, s_privatedata.i2caddress[index], static_cast<std::uint8_t>(reg), s_privatedata.acquisition[index].data, cbk);
                
        return resOK;        
    }
    

    
//    static void s_powerOFF(void)
//    {
//        embot::hw::gpio::set(gpioBOOT, embot::hw::gpio::State::RESET);
//        embot::hw::gpio::set(gpioRESET, embot::hw::gpio::State::RESET);        
//    }
    
    static void s_powerON(ADS122C04 s, embot::core::relTime waittime)
    {        
        embot::hw::gpio::set(embot::hw::bsp::ads122c04::getBSP().getPROP(s)->boot, embot::hw::gpio::State::SET);
        embot::hw::gpio::set(embot::hw::bsp::ads122c04::getBSP().getPROP(s)->reset, embot::hw::gpio::State::SET);
        
        embot::hw::sys::delay(waittime);         
    }
    
//    static result_t s_programregister_safe(ADS122C04 s, std::uint8_t reg, std::uint8_t val, embot::core::relTime timeout)
//    {
//        std::uint8_t index = embot::core::tointegral(s);   
//        embot::core::Data data(&val, 1);        
//        result_t r = embot::hw::i2c::write(s_privatedata.config[index].i2cdes.bus, s_privatedata.i2caddress[index], reg, data, timeout);                
//        return r;
//    }

    
    static result_t s_programregister(ADS122C04 s, std::uint8_t reg, std::uint8_t val, embot::core::relTime timeout)
    {
        std::uint8_t index = embot::core::tointegral(s);   
        s_privatedata.acquisition[index].startwrite(val);        
        result_t r = embot::hw::i2c::write(s_privatedata.config[index].i2cdes.bus, s_privatedata.i2caddress[index], reg, s_privatedata.acquisition[index].data, timeout);  
        s_privatedata.acquisition[index].stop();        
        return r;
    }
    
    
    static result_t s_writeregister(ADS122C04 s, std::uint8_t reg, std::uint8_t val, const embot::core::Callback &oncompletion)
    {
        std::uint8_t index = embot::core::tointegral(s);   
        s_privatedata.acquisition[index].startwrite(val, oncompletion);   
        embot::core::Callback cbk(s_sharedCBK, &s_privatedata.acquisition[index]);        
        result_t r = embot::hw::i2c::write(s_privatedata.config[index].i2cdes.bus, s_privatedata.i2caddress[index], reg, s_privatedata.acquisition[index].data, cbk);  
      
        return r;
    }
    
    static void s_sharedCBK(void *p)
    {
        Acquisition *acq = reinterpret_cast<Acquisition*>(p);                
        acq->stop();
    }
    

    
}}} // namespace embot { namespace hw { namespace ADS122C04 {



#endif //defined(EMBOT_ADS122C04_ENABLED)


    



// - end-of-file (leave a blank line after)----------------------------------------------------------------------------

