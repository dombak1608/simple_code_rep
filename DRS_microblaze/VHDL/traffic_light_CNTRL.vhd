library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity traffic_light_CNTRL is
Port ( 
	clk : in STD_LOGIC;
	segment : out STD_LOGIC_VECTOR (0 TO 7);
	display : out STD_LOGIC_VECTOR (3 DOWNTO 0)
	);
end traffic_light_CNTRL;

architecture Behavioral of traffic_light_CNTRL is

	CONSTANT timeR : INTEGER := 5;
	CONSTANT timeRY : INTEGER := 1;
	CONSTANT timeG : INTEGER := 4;
	CONSTANT timeY : INTEGER := 2;
	CONSTANT timeMAX : INTEGER := 5;
	
	TYPE state IS (stateR, stateRY, stateG, stateY);
	SIGNAL current_state, next_state: state;
	SIGNAL clk_div: STD_LOGIC;
	SIGNAL time_temp : INTEGER RANGE 0 TO timeMAX;

begin
	display <= "1110";
	
	clk_1Hz : entity work.generic_divider generic map (100_000_000) port map (clk, clk_div);
	process(clk_div)
		variable counter : INTEGER RANGE 0 TO timeMAX;
	begin
		if(clk_div'event and clk_div='1') then		
			counter := counter+1;
			if (counter=time_temp) then
				current_state <= next_state;
				counter := 0;
			end if;
		end if;
	end process;
	
	process(current_state)
	begin
		case current_state is
			when stateR => 
				segment <= "01111111";
				time_temp <= timeR;
				next_state <= stateRY;
			when stateRY => 
				segment <= "01111101";
				time_temp <= timeRY;
				next_state <= stateG;
			when stateG => 
				segment <= "11101111";
				time_temp <= timeG;
				next_state <= stateY;
			when stateY => 
				segment <= "11111101";
				time_temp <= timeY;
				next_state <= stateR;
		end case;
	end process;

end Behavioral;
