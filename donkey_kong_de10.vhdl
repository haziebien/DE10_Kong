-------------------------------------
--	Final Project
-- Donkey Kong Idea
-- Author: ZRM
--	Contributors: ()
-- Date: 11/12/24
-- Modified: 11/12/24
------------------------------------
--
-- Donkey kong game, using VGA controller, 
-- PIO, and accelerometer. 
--
--
------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity donkey_kong_de10 is 
	port(
			--------------------------
			-- General ports
			--------------------------
			CLOCK_50		: in std_logic;
			
--			--------------------------
--			-- PIO ports
--			--------------------------
			SW				: in std_logic_vector(9 downto 0);
			LEDR			: out std_logic_vector(9 downto 0);
			HEX0 			: out std_logic_vector(7 downto 0);
			HEX1			: out std_logic_vector(7 downto 0);
			
			--------------------------
			-- DRAM ports
			--------------------------
			DRAM_ADDR 	: out std_logic_vector(12 downto 0);
			DRAM_BA	 	: out std_logic_vector(1 downto 0);
			DRAM_CAS_N 	: out std_logic;
			DRAM_CKE   	: out std_logic;
			DRAM_CS_N  	: out std_logic;
			DRAM_RAS_N 	: out std_logic;
			DRAM_WE_N  	: out std_logic;
			DRAM_DQ    	: inout std_logic_vector(15 downto 0);
			DRAM_UDQM  	: out std_logic;
			DRAM_LDQM  	: out std_logic;			
			DRAM_CLK		: out std_logic;
			--------------------------
			-- VGA ports
			--------------------------
			VGA_HS		: out std_logic;
			VGA_VS		: out std_logic;
			VGA_R			: out std_logic_vector(3 downto 0);
			VGA_G			: out std_logic_Vector(3 downto 0);
			VGA_B			: out std_logic_vector(3 downto 0);
			
			
--			--------------------------
--			-- accelerometer
--			--------------------------
			G_SENSOR_SCLK		: out std_logic;
			G_SENSOR_SDI		: inout std_logic;
			G_SENSOR_CS_N		: out std_logic;
			G_SENSOR_INT		: in std_logic_vector(1 downto 0)
		
		);
end entity; 

architecture behavioral of donkey_kong_de10 is 
	--------------------------------------------
	-- signals
	--------------------------------------------
	signal seven_seg_sig : std_logic_vector(15 downto 0);
	
	--------------------------------------------
	-- component declaration
	--------------------------------------------
    component donkey_kong_qsys is
        port (
            accelerometer_spi_0_external_interface_I2C_SDAT      : inout std_logic                     := 'X';             -- I2C_SDAT
            accelerometer_spi_0_external_interface_I2C_SCLK      : out   std_logic;                                        -- I2C_SCLK
            accelerometer_spi_0_external_interface_G_SENSOR_CS_N : out   std_logic;                                        -- G_SENSOR_CS_N
            accelerometer_spi_0_external_interface_G_SENSOR_INT  : in    std_logic                     := 'X';             -- G_SENSOR_INT
            clk_clk                                              : in    std_logic                     := 'X';             -- clk
            clk_sdram_clk                                        : out   std_logic;                                        -- clk
            hex_pio_external_connection_export                   : out   std_logic_vector(15 downto 0);                    -- export
            led_pio_external_connection_export                   : out   std_logic_vector(7 downto 0);                     -- export
            reset_reset_n                                        : in    std_logic                     := 'X';             -- reset_n
            sdram_wire_addr                                      : out   std_logic_vector(12 downto 0);                    -- addr
            sdram_wire_ba                                        : out   std_logic_vector(1 downto 0);                     -- ba
            sdram_wire_cas_n                                     : out   std_logic;                                        -- cas_n
            sdram_wire_cke                                       : out   std_logic;                                        -- cke
            sdram_wire_cs_n                                      : out   std_logic;                                        -- cs_n
            sdram_wire_dq                                        : inout std_logic_vector(15 downto 0) := (others => 'X'); -- dq
            sdram_wire_dqm                                       : out   std_logic_vector(1 downto 0);                     -- dqm
            sdram_wire_ras_n                                     : out   std_logic;                                        -- ras_n
            sdram_wire_we_n                                      : out   std_logic;                                        -- we_n
            sw_pio_external_connection_export                    : in    std_logic_vector(7 downto 0)  := (others => 'X'); -- export
            vga_out_CLK                                          : out   std_logic;                                        -- CLK
            vga_out_HS                                           : out   std_logic;                                        -- HS
            vga_out_VS                                           : out   std_logic;                                        -- VS
            vga_out_BLANK                                        : out   std_logic;                                        -- BLANK
            vga_out_SYNC                                         : out   std_logic;                                        -- SYNC
            vga_out_R                                            : out   std_logic_vector(3 downto 0);                     -- R
            vga_out_G                                            : out   std_logic_vector(3 downto 0);                     -- G
            vga_out_B                                            : out   std_logic_vector(3 downto 0)                      -- B
        );
    end component donkey_kong_qsys;
		  
		  begin
		  
	----------------------------------------------
	-- component instantiation
	----------------------------------------------
    u0 : component donkey_kong_qsys
        port map (
            -- Accelerometer
				accelerometer_spi_0_external_interface_I2C_SDAT      => G_SENSOR_SDI,      -- accelerometer_spi_0_external_interface.I2C_SDAT
            accelerometer_spi_0_external_interface_I2C_SCLK      => G_SENSOR_SCLK,      --                                       .I2C_SCLK
            accelerometer_spi_0_external_interface_G_SENSOR_CS_N => G_SENSOR_CS_N, --                                       .G_SENSOR_CS_N
            accelerometer_spi_0_external_interface_G_SENSOR_INT  => G_SENSOR_INT(1),  --                                       .G_SENSOR_INT
            
				-- CLK
				clk_clk                                              => CLOCK_50,                                              --                                    clk.clk
            clk_sdram_clk                                        => DRAM_CLK,                                        --                              clk_sdram.clk
            
				-- HEX io
				hex_pio_external_connection_export                   => seven_seg_sig,                   --            hex_pio_external_connection.export
            
				-- LED IO
				led_pio_external_connection_export                   => LEDR(7 downto 0),                   --            led_pio_external_connection.export
            
				-- Reset
				reset_reset_n                                        => '1',                                        --                                  reset.reset_n
            
				-- SDRAM
				sdram_wire_addr                                      => DRAM_ADDR,                                      --                             sdram_wire.addr
            sdram_wire_ba                                        => DRAM_BA,                                        --                                       .ba
            sdram_wire_cas_n                                     => DRAM_CAS_N,                                     --                                       .cas_n
            sdram_wire_cke                                       => DRAM_CKE,                                       --                                       .cke
            sdram_wire_cs_n                                      => DRAM_CS_N,                                      --                                       .cs_n
            sdram_wire_dq                                        => DRAM_DQ,                                        --                                       .dq
            sdram_wire_dqm(1)                                    => DRAM_UDQM,                                       --                                       .dqm
            sdram_wire_dqm(0)												  => DRAM_LDQM,
				sdram_wire_ras_n                                     => DRAM_RAS_N,                                     --                                       .ras_n
            sdram_wire_we_n                                      => DRAM_WE_N,                                      --                                       .we_n
            
				-- switch IO
				sw_pio_external_connection_export                    => sw(7 downto 0),                    --             sw_pio_external_connection.export
            -- vga
				--vga_out_CLK                                          => CONNECTED_TO_vga_out_CLK,                                          --                                vga_out.CLK
            vga_out_HS                                           => VGA_HS,                                           --                                       .HS
            vga_out_VS                                           => VGA_VS,                                           --                                       .VS
            --vga_out_BLANK                                        => CONNECTED_TO_vga_out_BLANK,                                        --                                       .BLANK
            --vga_out_SYNC                                         => CONNECTED_TO_vga_out_SYNC,                                         --                                       .SYNC
            vga_out_R                                            => VGA_R,                                            --                                       .R
            vga_out_G                                            => VGA_G,                                            --                                       .G
            vga_out_B                                            => VGA_B                                             --                                       .B
        );

	
	----------------------------------------------
	-- QSYS
	----------------------------------------------
	
	----------------------------------------------
	-- Misc Assignments
	----------------------------------------------
	HEX1 <= seven_seg_sig(15 downto 8);
	HEX0 <= seven_seg_sig(7 downto 0);
	
end architecture; 

	