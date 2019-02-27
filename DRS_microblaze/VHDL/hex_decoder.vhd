library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity hex_decoder is
    Port ( number_in : in  STD_LOGIC_VECTOR(3 DOWNTO 0);
           segment_out : out  STD_LOGIC_VECTOR (7 downto 0)
			  );
end hex_decoder;

architecture Behavioral of hex_decoder is

begin
	segment_out	<=		"00000011"	WHEN	number_in = "0000" ELSE	--0
						"10011111"	WHEN	number_in = "0001" ELSE --1
						"00100101"	WHEN	number_in = "0010" ELSE --2
						"00001101"	WHEN	number_in = "0011" ELSE --3
						"10011001"	WHEN	number_in = "0100" ELSE --4
						"01001001"	WHEN	number_in = "0101" ELSE --5
						"11000001"	WHEN	number_in = "0110" ELSE --6
						"00011111"	WHEN	number_in = "0111" ELSE --7
						"00000001"	WHEN	number_in = "1000" ELSE --8
						"00011001"	WHEN	number_in = "1001" ELSE --9
						"00010001"	WHEN	number_in = "1010" ELSE --A
						"11000001"	WHEN	number_in = "1011" ELSE --B
						"01100011"	WHEN	number_in = "1100" ELSE --C
						"10000101"	WHEN	number_in = "1101" ELSE --D
						"01100001"	WHEN	number_in = "1110" ELSE --E
						"01110001";								--F

end Behavioral;

