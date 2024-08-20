/**
 * @file    Ext_Flash.c  (formerly w25.c)
 * @brief   Board layer Driver for the Micron W25 Serial Multi-I/O Flash Memory.
 */

/******************************************************************************
 *
 * Copyright (C) 2022-2023 Maxim Integrated Products, Inc. All Rights Reserved.
 * (now owned by Analog Devices, Inc.),
 * Copyright (C) 2023-2024 Analog Devices, Inc. All Rights Reserved. This software
 * is proprietary to Analog Devices, Inc. and its licensors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * The W25N02KV2G-bit memory array is organized into 131,072programmable 
 * pages of 2,048-bytes each. The entire page can be programmed at one time 
 * using the data from the 2,048-Byte internal buffer. Pages can be erased in 
 * groups of 64(128KB blockerase). The W25N02KVhas 2,048erasable blocks.
 ******************************************************************************/

/* **** Includes **** */
#include <stdint.h>
#include <stddef.h>
#include "Ext_Flash.h"
#include "mxc_delay.h"

/**
 * @ingroup w25
 * @{
 */

/* **** Definitions **** */
#define W25N0_ID_LEN (4)

#define W25N0_WIP_MASK 0x01 /**< Status Reg-3: Work In Progress          */
#define W25N0_ERASE_FAIL_MASK 0x02 /**< Status Reg-3: Erase Failure      */
#define W25N0_WEL_MASK 0x02 /**< Status Reg-3: Write Enable Latch mask   */
#define W25N0_QE_MASK 0x02 /**< Status Reg-2: Quad-SPI enable mask      */

#define W25N0_TB_POS 5
#define W25N0_TB_MASK (1 << W25N0_TB_POS) /**< Top/Bottom Select mask         */
#define W25N0_BP_POS 2
#define W25N0_BP_MASK (0x7 << W25N0_BP_POS) /**< Block Protect mask             */
#define W25N0_SR1_FP_MASK \
    (W25N0_TB_MASK | W25N0_BP_MASK) /**< Mask of all flash block protect bits in status register 1 */
#define W25N0_CMP_POS 6
#define W25N0_CMP_MASK (1 << W25N0_CMP_POS) /**< Flash protect complement bit mask */

#define W25N0_GET_BP_IN_FIRST_HALF(pg) \
    ((pg < 4)              ? 1 :     \
     (pg > 3 && pg < 8)    ? 2 :     \
     (pg > 7 && pg < 16)   ? 3 :     \
     (pg > 15 && pg < 32)  ? 4 :     \
     (pg > 31 && pg < 64)  ? 5 :     \
     (pg > 63 && pg < 128) ? 6 :     \
                             -1)
#define W25N0_GET_BP_IN_SECOND_HALF(pg) \
    ((pg < 192)             ? 5 :     \
     (pg > 191 && pg < 224) ? 4 :     \
     (pg > 223 && pg < 240) ? 3 :     \
     (pg > 239 && pg < 248) ? 2 :     \
     (pg > 247 && pg < 252) ? 1 :     \
                              -1)

#define W25N0_DEVICE_SIZE 0x10000000  //256 M-bytes (2 Gigabit)
#define W25N0_BLOCK_SIZE 0x20000   //128 kibibytes 131,072
#define W25N0_PAGE_SIZE 2048       //2048 bytes per page
#define W25N0_TOTAL_BLOCKS 2048    //2048 blocks

#define W25N0_CMD_RST_EN 0x66 /**< Reset Enable                   */
#define W25N0_CMD_RST_MEM 0x99 /**< Reset Memory                   */
#define W25N0_CMD_ID 0x9F /**< ID                             */
#define W25N0_CMD_WRITE_EN 0x06 /**< Write Enable                   */
#define W25N0_CMD_WRITE_DIS 0x04 /**< Write Disable                  */

#define W25N0_CMD_READ_SR 0x05 /**< Start command to Read Status Register         */
#define W25N0_PROT_REG             0xA0   /** Status Register 1 Address */
#define W25N0_CONFIG_REG           0xB0   /** Status Register 2 Address */
#define W25N0_STAT_REG             0xC0   /** Status Register 3 Address */

#define W25N0_CMD_WRITE_SR 0x01 /**< Start command to Write Status Register        */

#define W25N0_PROG_DATA_LOAD   0x02
#define W25N0_RAND_PROG_DATA_LOAD  0x84

#define W25N0_PROG_EXECUTE 0x10

#define W25N0_PAGE_DATA_READ       0x13
#define W25N0_READ                 0x03

#define W25N0_CMD_PPROG 0x02 /**< Page Program                   */
#define W25N0_CMD_QUAD_PROG 0X32 /**< Quad (4 x I/O) Page Program    */

#define W25N0_CMD_128K_ERASE 0xD8 /**< (128KB) Block Erase.  This is the only erase option  */


/* **** Globals **** */

static Ext_Flash_Config_t g_cfg;
static uint8_t g_is_configured = 0;

/* **** Static Functions **** */

/* ************************************************************************* */
static Ext_Flash_Error_t flash_busy()
{
    uint8_t buf;

    Ext_Flash_Read_SR(&buf, Ext_Flash_StatusReg_3);

    if (buf & W25N0_WIP_MASK) {
        return EF_E_BUSY;
    } else {
        return EF_E_SUCCESS;
    }
}

/* ************************************************************************* */
static Ext_Flash_Error_t is_erase_failed()
{
    uint8_t buf;

    Ext_Flash_Read_SR(&buf, Ext_Flash_StatusReg_3);

    if (buf & W25N0_ERASE_FAIL_MASK) {
        return EF_E_ERASE_FAILED;
    } else {
        return EF_E_SUCCESS;
    }
}

/* ************************************************************************* */
static Ext_Flash_Error_t write_enable()
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;
    uint8_t cmd = W25N0_CMD_WRITE_EN;
    uint8_t buf = 0;

    // Send the command
    if ((err = g_cfg.write(&cmd, 1, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    if ((err = Ext_Flash_Read_SR(&buf, Ext_Flash_StatusReg_3)) != EF_E_SUCCESS) {
        return err;
    }

    if (buf & W25N0_WEL_MASK) {
        return EF_E_SUCCESS;
    }

    return EF_E_BAD_STATE;
}

/* ************************************************************************* */
static Ext_Flash_Error_t inline read_reg(uint8_t cmd, uint8_t *buf)
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;

    if (!buf) {
        return EF_E_BAD_PARAM;
    }

    // Send the command
    if ((err = g_cfg.write(&cmd, 1, 0, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    // Read the data
    if ((err = g_cfg.read(buf, 1, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    return EF_E_SUCCESS;
}

/* ************************************************************************* */
static Ext_Flash_Error_t inline read_status_reg(uint8_t* cmd, uint8_t *buf)
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;

    if (!buf) {
        return EF_E_BAD_PARAM;
    }

    // Send the command
    if ((err = g_cfg.write(cmd, sizeof(cmd), 0, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    // Read the data
    if ((err = g_cfg.read(buf, 1, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    return EF_E_SUCCESS;
}

/* ************************************************************************* */
static Ext_Flash_Error_t inline write_reg(uint8_t *buf, unsigned len)
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;

    if (!buf || (len == 0)) {
        return EF_E_BAD_PARAM;
    }

    if ((err = write_enable()) != EF_E_SUCCESS) {
        return err;
    }

    // Send the command and data
    if ((err = g_cfg.write(buf, len, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    return EF_E_SUCCESS;
}

/* **** Functions **** */

Ext_Flash_Error_t Ext_Flash_Configure(Ext_Flash_Config_t *cfg)
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;

    if (cfg == NULL) {
        return EF_E_BAD_PARAM;
    }

    g_cfg = *cfg;
    g_is_configured = 1;

    return err;
}

/* ************************************************************************* */

Ext_Flash_Error_t Ext_Flash_Init(void)
{
    if (!g_is_configured) {
        return EF_E_BAD_STATE;
    }

    return g_cfg.init();
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Reset(void)
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;
    int busy_count = 0;
    uint8_t cmd = W25N0_CMD_RST_EN;

    // Send the Enable Reset command
    if ((err = g_cfg.write(&cmd, 1, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    // Send reset command
    cmd = W25N0_CMD_RST_MEM;
    if ((err = g_cfg.write(&cmd, 1, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    while (flash_busy()) {
        busy_count++;
        if (busy_count > 20000) {
            return EF_E_TIME_OUT;
        }
    }

    return EF_E_SUCCESS;
}

/* ************************************************************************* */
uint32_t Ext_Flash_ID(void)
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;
    uint8_t cmd = W25N0_CMD_ID;
    uint8_t id[W25N0_ID_LEN] = { 0 };

    // Send the command
    if ((err = g_cfg.write(&cmd, 1, 0, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    // Read the data
    if ((err = g_cfg.read(id, W25N0_ID_LEN, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    /* id[0] is a dummy value coming in  id[1] in mnf_id, id[2] and id[3] are chipID*/

    return ((uint32_t)(id[3] | (id[2] << 8) | (id[1] << 16)));
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Quad(int enable)
{
    int err = EF_E_SUCCESS;
    uint8_t pre_buf = 0;
    uint8_t post_buf = 0;

    // Enable QSPI mode
    if ((err = Ext_Flash_Read_SR(&pre_buf, Ext_Flash_StatusReg_2)) != EF_E_SUCCESS) {
        return err;
    }

    while (flash_busy()) {}

    if (enable) {
        if (pre_buf & W25N0_QE_MASK) {
            return EF_E_SUCCESS;
        }
        pre_buf |= W25N0_QE_MASK;
    } else {
        if (!(pre_buf & W25N0_QE_MASK)) {
            return EF_E_SUCCESS;
        }
        pre_buf &= ~W25N0_QE_MASK;
    }

    if (write_enable() != EF_E_SUCCESS) {
        return EF_E_BAD_STATE;
    }

    if ((err = Ext_Flash_Write_SR(pre_buf, Ext_Flash_StatusReg_2)) != EF_E_SUCCESS) {
        return err;
    }

    while (flash_busy()) {}

    if ((err = Ext_Flash_Read_SR(&post_buf, Ext_Flash_StatusReg_2)) != EF_E_SUCCESS) {
        return err;
    }

    while (flash_busy()) {}

    if (enable) {
        if (!(post_buf & W25N0_QE_MASK)) {
            return EF_E_ERROR;
        }
    } else {
        if (post_buf & W25N0_QE_MASK) {
            return EF_E_ERROR;
        }
    }

    return EF_E_SUCCESS;
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_DataRead(uint32_t address)
{
    int err = EF_E_SUCCESS;
    uint8_t cmd[4] = {W25N0_PAGE_DATA_READ, 0x00, 0x00, 0x00};

    while (flash_busy()) {
        MXC_Delay(1000);
    }

    cmd[2] = (uint8_t)((address & 0xFF00) >> 8);
    cmd[3] = (uint8_t)(address);
 
    // Send command & address 
    if ((err = g_cfg.write(&cmd[0], 4, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    volatile int timeout = 0;
    while (flash_busy()) {
        timeout++;
        if (timeout > 10000) {
            return EF_E_TIME_OUT;
        }
    }

    return EF_E_SUCCESS;
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Read(uint32_t address, uint8_t *rx_buf, uint32_t rx_len, Ext_Flash_DataLine_t d_line)
{
    int err = EF_E_SUCCESS;
    uint8_t cmd[4] = { W25N0_READ, 0x00, 0x00, 0x00};
    uint8_t dummy_bits = 0x00;

    if (flash_busy()) {
        MXC_Delay(1000);
    }

    cmd[1] = (uint8_t)((address & 0xFF00) >> 8);
    cmd[2] = (uint8_t)(address & 0xFF);
    cmd[3] = dummy_bits;


    // Send command & address & dummy bits
    if ((err = g_cfg.write(&cmd[0], 4, 0, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    if ((err = g_cfg.read(rx_buf, rx_len, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    return EF_E_SUCCESS;
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Program_Page(uint32_t address, uint8_t *tx_buf, uint32_t tx_len,
                           Ext_Flash_DataLine_t d_line)
{
    int err = EF_E_SUCCESS;
    volatile int timeout = 0;
    uint8_t cmd[3] = {W25N0_PROG_DATA_LOAD, 0x00, 0x00};
    uint32_t len = 0;
    uint32_t next_page = 0;
    uint8_t *pWrite_Data = NULL;

    if (tx_buf == NULL) {
        return EF_E_BAD_PARAM;
    }

    // if flash address is out-of-range
    if ((address >= W25N0_DEVICE_SIZE) || ((address + tx_len) > W25N0_DEVICE_SIZE)) {
        return EF_E_BAD_PARAM; // attempt to write outside flash memory size
    }

    pWrite_Data = tx_buf; // note our starting source data address

    if (flash_busy()) {
        return EF_E_BUSY;
    }

    // Now write out as many pages of flash as required to fulfil the request
    while (tx_len > 0) {
        while (write_enable()) {
            timeout++;
            if (timeout > 100) {
                return EF_E_TIME_OUT;
            }
        }

        cmd[1] = (uint8_t)((address & 0xFF00) >> 8);
        cmd[2] = (uint8_t)(address & 0xff);

        // Send the command and dummy bits
        if (d_line == Ext_Flash_DataLine_Quad) {
            cmd[0] = W25N0_CMD_QUAD_PROG;
        } else {
            cmd[0] = W25N0_CMD_PPROG;
        }

        if ((err = g_cfg.write(&cmd[0], 1, 0, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
            return err;
        }

        // Send the address
        if ((err = g_cfg.write(&cmd[1], 2, 0, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
            return err;
        }

        // calculate the next flash page boundary from our starting address
        next_page = ((address & ~(W25N0_PAGE_SIZE - 1)) + W25N0_PAGE_SIZE);

        // Now check for how much data to write on this page of flash
        if ((address + tx_len) < next_page) {
            len = tx_len; // no page boundary is crossed
        } else {
            len = next_page - address; // adjust length of this write to say within the current page
        }

        // Write the data
        if ((err = g_cfg.write(pWrite_Data, len, 1, d_line)) != EF_E_SUCCESS) {
            return err;
        }

        if (tx_len >= len) {
            tx_len -= len; // what's left to write
        }

        // if there is more to write
        if (tx_len > 0) {
            address += len; // calculate new starting flash_address
            pWrite_Data += len; // and source data address
        }

        timeout = 0;
        while (flash_busy()) {
            timeout++;
            if (timeout > 10000) {
                return EF_E_TIME_OUT;
            }
        }

        //Transfer data from buffer to flash memory
        if(EF_E_SUCCESS !=  Ext_Flash_Prog_Execute(address))
        {
            return EF_E_ERROR;
        }
    }
    return EF_E_SUCCESS;
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Unprotect_StatusRegister(void)
{
    uint8_t cmd[3] = {W25N0_CMD_WRITE_SR, W25N0_PROT_REG, 0x00};

    return write_reg(cmd, sizeof(cmd));
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Bulk_Erase(void)
{
    int err = EF_E_SUCCESS;
    int blockCount = 0;
 
    volatile int timeout = 0;

    if (flash_busy()) {
        return EF_E_BUSY;
    }

    if (write_enable() != 0) {
        return EF_E_BAD_STATE;
    }

    // Erase block by block until reaching total blocks
    while(W25N0_TOTAL_BLOCKS > blockCount){
        if (Ext_Flash_Erase(0x00000, W25N0_CMD_128K_ERASE) != EF_E_SUCCESS){
            return err;
        }
        timeout = 0;       

        while (flash_busy()){
            timeout++;
            if (timeout > 100000000){
                return EF_E_TIME_OUT;
            }
        }
        blockCount++;
    }
    return EF_E_SUCCESS;
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Erase(uint32_t address, Ext_Flash_Erase_t size)
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;
    uint8_t cmd[4] = { 0 };
    volatile int timeout = 0;

    if (flash_busy()) {
        return EF_E_BUSY;
    }

    if (write_enable() != 0) {
        return EF_E_BAD_STATE;
    }

    switch (size) {
    // case Ext_Flash_Erase_4K:
    // default:
    //     cmd[0] = W25N0_CMD_4K_ERASE;
    //     break;
    // case Ext_Flash_Erase_32K:
    //     cmd[0] = W25N0_CMD_32K_ERASE;
    //     break;
    // case Ext_Flash_Erase_64K:
    //     cmd[0] = W25N0_CMD_64K_ERASE;
    //     break;
    case Ext_Flash_Erase_128K:
    default:
        cmd[0] = W25N0_CMD_128K_ERASE;
        break;
    }

    cmd[1] = 0x00;
    cmd[2] = (uint8_t)((address & 0xFF00) >> 8);
    cmd[3] = (uint8_t)address;


    // Send the command and the address
    if ((err = g_cfg.write(&cmd[0], 4, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    while (flash_busy()) {
        timeout++;
        if (timeout > 1000000000) {
            return EF_E_TIME_OUT;
        }
    }

    if(EF_E_SUCCESS != is_erase_failed()){
        return EF_E_ERASE_FAILED;
    }

    return EF_E_SUCCESS;
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Read_SR(uint8_t *buf, Ext_Flash_StatusReg_t reg_num)
{
    uint8_t cmd[3] = {W25N0_CMD_READ_SR, 0x00, 0x00};

    if (buf == NULL) {
        return EF_E_BAD_PARAM;
    }

    switch (reg_num) {
    case Ext_Flash_StatusReg_1:
        cmd[1] = W25N0_PROT_REG;
        break;
    case Ext_Flash_StatusReg_2:
        cmd[1] = W25N0_CONFIG_REG;
        break;
    case Ext_Flash_StatusReg_3:
        cmd[1] = W25N0_STAT_REG;
        break;
    default:
        return EF_E_BAD_PARAM;
    }

    return read_status_reg(cmd, buf);
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Write_SR(uint8_t value, Ext_Flash_StatusReg_t reg_num)
{
    uint8_t cmd[3] = {W25N0_CMD_WRITE_SR, 0x00, value};

    switch (reg_num) {
    case Ext_Flash_StatusReg_1:
        cmd[1] = W25N0_PROT_REG;
        break;
    case Ext_Flash_StatusReg_2:
        cmd[1] = W25N0_CONFIG_REG;
        break;
    default:
        return EF_E_BAD_PARAM;
    }

    return write_reg(cmd, sizeof(cmd));
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Prog_Execute(uint32_t address)
{
    int err = EF_E_SUCCESS;
    volatile int timeout = 0;
    uint8_t cmd[4] = {W25N0_PROG_EXECUTE, 0x00, 0x00, 0x00};

    cmd[2]  = (uint8_t)((address & 0xFF00) >> 8);
    cmd[3]  = (uint8_t)address;

    // Send the command and the address
    if ((err = g_cfg.write(&cmd[0], 4, 1, Ext_Flash_DataLine_Single)) != EF_E_SUCCESS) {
        return err;
    }

    while (flash_busy()) {
        timeout++;
        if (timeout > 1000000000) {
            return EF_E_TIME_OUT;
        }
    }

    if(EF_E_SUCCESS != is_erase_failed()){
        return EF_E_ERASE_FAILED;
    }

    return EF_E_SUCCESS;
}

/* ************************************************************************* */
Ext_Flash_Error_t Ext_Flash_Block_WP(uint32_t addr, uint32_t begin)
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;
    uint8_t sr1, sr2, page_num, bp, cmp;

    if (addr >= W25N0_DEVICE_SIZE) { // Check address valid
        return EF_E_ERROR;
    }

    page_num = addr / W25N0_BLOCK_SIZE; // Get page number in which "addr" is located
    if ((begin && page_num > 251) || (!begin && page_num < 4)) { // Entire memory array to be locked
        begin = 0;
        cmp = 1;
        bp = 0;
    } else if ((begin && addr == 0) ||
               (!begin && addr == W25N0_DEVICE_SIZE)) { // Entire memory array to be unlocked
        begin = 0;
        cmp = 0;
        bp = 0;
    } else if ((begin && page_num < 128) ||
               (!begin &&
                page_num > 127)) { // If address in the same half of flash as the starting point
        cmp = 0;

        if (!begin) {
            page_num = 255 - page_num;
        }

        bp = W25N0_GET_BP_IN_FIRST_HALF(page_num);
    } else { // If address in the opposite half of flash as the starting point
        cmp = 1;

        if (!begin) {
            page_num = 255 - page_num;
        }

        bp = W25N0_GET_BP_IN_SECOND_HALF(page_num);
    }

    if ((err = Ext_Flash_Read_SR(&sr1, Ext_Flash_StatusReg_1)) !=
        EF_E_SUCCESS) { // Read current value of flash protect bits
        return err;
    }
    if ((err = Ext_Flash_Read_SR(&sr2, Ext_Flash_StatusReg_2)) != EF_E_SUCCESS) {
        return err;
    }

    sr1 = (sr1 & ~W25N0_SR1_FP_MASK) | (!!begin << W25N0_TB_POS) |
          (bp << W25N0_BP_POS); // Modify flash protect bits
    sr2 = (sr2 & ~W25N0_CMP_MASK) | (cmp << W25N0_CMP_POS);

    if ((err = Ext_Flash_Write_SR(sr1, Ext_Flash_StatusReg_1)) !=
        EF_E_SUCCESS) { // Write flash protect settings back to W25
        return err;
    }
    if ((err = Ext_Flash_Write_SR(sr2, Ext_Flash_StatusReg_2)) != EF_E_SUCCESS) {
        return err;
    }

    return EF_E_SUCCESS;
}

/* ************************************************************************* */
Ext_Flash_Unblk_t Ext_Flash_GetAvailableFlash(void)
{
    Ext_Flash_Error_t err = EF_E_SUCCESS;
    uint8_t sr1, sr2;
    uint32_t page_addr;
    Ext_Flash_Unblk_t free_flash;

    if ((err = Ext_Flash_Read_SR(&sr1, Ext_Flash_StatusReg_1)) !=
        EF_E_SUCCESS) { // Get current value of flash protect bits
        free_flash.start_addr = err;
        free_flash.end_addr = err;
        return (free_flash);
    }
    if ((err = Ext_Flash_Read_SR(&sr2, Ext_Flash_StatusReg_2)) != EF_E_SUCCESS) {
        free_flash.start_addr = err;
        free_flash.end_addr = err;
        return (free_flash);
    }

    // Use CMP, TB, and BP bits to find start and end addresses
    if (((sr1 & W25N0_BP_MASK) >> W25N0_BP_POS) == 0) { // Special case: block protect == 0
        if (sr2 & W25N0_CMP_MASK) {
            free_flash.start_addr = 0; // CMP == 1 ---> All mem protected
            free_flash.end_addr = 0;
        } else {
            free_flash.start_addr = 0; // CMP == 0 ---> All mem un-protected
            free_flash.end_addr = W25N0_DEVICE_SIZE;
        }
    } else if (((sr1 & W25N0_BP_MASK) >> W25N0_BP_POS) == 7) { // Special case: block protect == 7
        if (sr2 & W25N0_CMP_MASK) {
            free_flash.start_addr = 0; // CMP == 1 ---> All mem un-protected
            free_flash.end_addr = W25N0_DEVICE_SIZE;
        } else { // CMP == 0 ---> All mem protected
            free_flash.start_addr = 0;
            free_flash.end_addr = 0;
        }
    } else { // 0 < Block Protect < 7
        page_addr = 4 << (((sr1 & W25N0_BP_MASK) >> W25N0_BP_POS) -
                          1); // Find page offset from value of block protect field

        if ((sr1 & W25N0_TB_MASK) && (sr2 & W25N0_CMP_MASK)) { // TB == 1 && CMP == 1
            free_flash.start_addr = 0;
            free_flash.end_addr = page_addr * W25N0_BLOCK_SIZE;
        } else if ((sr1 & W25N0_TB_MASK) && !(sr2 & W25N0_CMP_MASK)) { // TB == 1 && CMP == 0
            free_flash.start_addr = page_addr * W25N0_BLOCK_SIZE;
            free_flash.end_addr = W25N0_DEVICE_SIZE;
        } else if (!(sr1 & W25N0_TB_MASK) && (sr2 & W25N0_CMP_MASK)) { // TB == 0 && CMP == 1
            free_flash.start_addr = 0;
            free_flash.end_addr = (256 - page_addr) * W25N0_BLOCK_SIZE;
        } else { // TB == 0 && CMP == 0
            free_flash.start_addr = (256 - page_addr) * W25N0_BLOCK_SIZE;
            free_flash.end_addr = W25N0_DEVICE_SIZE;
        }
    }

    return free_flash;
}
/**@} end of ingroup w25 */
