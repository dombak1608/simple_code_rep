library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity moving_light is
Port ( 
	clk : in STD_LOGIC;
	output : out STD_LOGIC_VECTOR (7 DOWNTO 0)
	 );
end moving_light;

architecture Behavioral of moving_light is

	TYPE state IS (state0, state1, state2, state3, state4, state5, state6, state7);
	SIGNAL current_state, next_state: state;
	SIGNAL clk_div: STD_LOGIC;

begin

	clk_1Hz : entity work.generic_divider generic map (100_000_000) port map (clk, clk_div);
	
	process(clk_div)
	begin
		if(clk_div'event and clk_div='1') then
			current_state <= next_state;
		end if;
	end process;
	
	process(current_state)
	begin
		case current_state is
			when state0 =>
				output <= "00000001";
				next_state <= state1;
			when state1 =>
				output <= "00000010";
				next_state <= state2;
			when state2 =>
				output <= "00000100";
				next_state <= state3;
			when state3 =>
				output <= "00001000";
				next_state <= state4;
			when state4 =>
				output <= "00010000";
				next_state <= state5;
			when state5 =>
				output <= "00100000";
				next_state <= state6;
			when state6 =>
				output <= "01000000";
				next_state <= state7;
			when state7 =>
				output <= "10000000";
				next_state <= state0;
		end case;
	end process;
end Behavioral;