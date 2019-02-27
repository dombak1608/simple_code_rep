library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity LEDMatrixCNTRL is

PORT(
	rst, wake, send_data, send_reg, clk_in : in STD_LOGIC;
	data : in STD_LOGIC_VECTOR (0 to 63);
	CNTRL_reg : in STD_LOGIC_VECTOR (0 to 15);
	load, d_out : out STD_LOGIC;
	clk_out : buffer STD_LOGIC
	);

end LEDMatrixCNTRL;

architecture Behavioral of LEDMatrixCNTRL is

	SIGNAL decoded_data: STD_LOGIC_VECTOR (0 TO 15);
	SIGNAL decode: INTEGER RANGE 0 TO 7;
	CONSTANT wake_signal : STD_LOGIC_VECTOR(0 TO 15) := "0000110000000001";
	CONSTANT shutDown_signal : STD_LOGIC_VECTOR(0 TO 15) := "0000110000000000";
	TYPE state IS (ShutDown, WakeState, SSD, NormalOP, SendData, SendReg);
	SIGNAL current_state, next_state: state;
	SIGNAL sent, sentMatrix: STD_LOGIC;
	

begin
	PROCESS(clk_in)
		variable temp : INTEGER RANGE 0 TO 128;
	BEGIN
		IF(clk_in'event and clk_in='1') THEN
			temp := temp+1;
			IF (temp >= 128) THEN
				clk_out <= not clk_out;
				temp := 0;
			END IF;
		END IF;
	END PROCESS;

	FSMSequential: process(clk_out, rst)
	begin
		if(rst='1') then
			current_state <= SSD;
		elsif(clk_out'event and clk_out='1') then
			current_state <= next_state;
		end if;
	end process;

	process(current_state, wake, send_data, send_reg, sent, sentMatrix)
	begin
		case current_state is
			when ShutDown =>
				if(wake='1') then
					next_state <= WakeState;
				else 
					next_state <= ShutDown;
				end if;
			when WakeState =>
				if(sent='1') then
					next_state <= NormalOP;
				else
					next_state <= WakeState;
				end if;
			when SSD =>
				if(sent='1') then
					next_state <= ShutDown;
				else
					next_state <= SSD;
				end if;
			when NormalOP =>
				if(send_reg='1') then
					next_state <= SendReg;
				elsif(send_data='1') then
					next_state <= SendData;
				else
					next_state <= SendReg;
				end if;
			when SendReg =>
				if(sent='1') then
					next_state <= NormalOP;
				else
					next_state <= SendReg;
				end if;
			when SendData =>
				if(sentMatrix='1') then
					next_state <= NormalOP;
				else
					next_state <= SendData;
				end if;
			end case;
	end process;
	
	setLoad: process(clk_out,rst)
	begin
		if (rst='1') then
			load <= '0';
		elsif(clk_out'event and clk_out='1') then
			if (sent='1') then
				load <= '1';
			else
				load <= '0';
			end if;
		end if;
	end process;

	SendProcess: process(rst, clk_out)
	VARIABLE counter_reg: INTEGER RANGE 0 TO 16;
	VARIABLE counter_data: INTEGER RANGE 0 TO 8;
	
	begin
		if(rst='1') then
			sent <= '0';
			sentMatrix <= '0';
			d_out <= '0';
			counter_reg := 0;
			counter_data := 0;
		elsif(clk_out'event AND clk_out = '0') then		
			case current_state IS
				when ShutDown =>
					sent <= '0';
					counter_reg := 0;
					counter_data := 0;					
				when WakeState =>
					d_out <= wake_signal(counter_reg);
					counter_reg := counter_reg+1;
					if(counter_reg>=16) then
						sent <= '1';
						counter_reg := 0;
					end if;					
				when SSD =>
					d_out <= shutDown_signal(counter_reg);
					counter_reg := counter_reg+1;
					if(counter_reg>=16) then
						sent <= '1';
						counter_reg := 0;
					end if;
				when NormalOP =>
					sent <= '0';
					sentMatrix <= '0';
					counter_reg := 0;
					counter_data := 0;					
				when SendReg =>
					d_out <= CNTRL_reg(counter_reg);
					counter_reg := counter_reg+1;
					if(counter_reg>=16) then
						sent <= '1';
						counter_reg := 0;
					end if;					
				when SendData =>
					d_out <= decoded_data(counter_reg);
					counter_reg := counter_reg+1;
					if(counter_reg>=16) then
						sent <= '1';
						counter_reg := 0;
						counter_data := counter_data+1;						
					else
						sent <= '0';
					end if;
					if(counter_data>=8) then
						sentMatrix <= '1';
						counter_data := 0;
						
					end if;					
			end case;		
		end if;
		
		decode <= counter_data;

	end process;
	
	decoded_data <= "0000" & "0001" & data(0 to 7) when decode = 0 else
						"0000" & "0010" & data(8 to 15) when decode = 1 else
						"0000" & "0011" & data(16 to 23) when decode = 2 else
						"0000" & "0100" & data(24 to 31) when decode = 3 else
						"0000" & "0101" & data(32 to 39) when decode = 4 else
						"0000" & "0110" & data(40 to 47) when decode = 5 else
						"0000" & "0111" & data(48 to 55) when decode = 6 else
						"0000" & "1000" & data(56 to 63);
end Behavioral;

