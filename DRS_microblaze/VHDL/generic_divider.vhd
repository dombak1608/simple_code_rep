library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity generic_divider is
GENERIC
(
	N	:	INTEGER	:=	50_000_000
);
PORT
(
	clk_in	:	IN	STD_LOGIC;
	clk_out	:	OUT STD_LOGIC
);
end generic_divider;

architecture Behavioral of generic_divider is
	signal clk_t	:	STD_LOGIC;
begin

PROCESS(clk_in)
	variable temp	:	INTEGER RANGE 0 TO N/2;
BEGIN
	
	IF(clk_in'event and clk_in = '1') THEN
		temp := temp+1;
		IF(temp >= N/2) THEN
			clk_t <= not clk_t;
			temp := 0;
		END IF;
	END IF;
	
END PROCESS;

clk_out <= clk_t;

end Behavioral;

