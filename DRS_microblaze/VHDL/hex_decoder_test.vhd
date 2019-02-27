library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity hex_decoder_test is
    Port ( number_in : in  STD_LOGIC_VECTOR(3 DOWNTO 0);
           segment_out : out  STD_LOGIC_VECTOR (7 downto 0);
           display_out : out  STD_LOGIC_VECTOR (3 downto 0)
			  );
end hex_decoder_test;

architecture Behavioral of hex_decoder_test is

begin
	decoder	:	entity work.hex_decoder port map(number_in, segment_out);
	
	display_out <= "0011";
end Behavioral;

