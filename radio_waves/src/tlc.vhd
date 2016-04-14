library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
--use IEEE.STD_LOGIC_ARITH.ALL;
--use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.NUMERIC_STD.ALL;

entity TLC is
    Port (
        -- The crystal:
        CLK     : in    STD_LOGIC;
        -- Switches:
        SW      : in    STD_LOGIC_VECTOR ( 7 downto 0);
        -- LED:
        LED     : out   STD_LOGIC_VECTOR ( 7 downto 0) := x"00";
        -- Jack A
        JA      : out   STD_LOGIC_VECTOR ( 7 downto 0) := x"00";
        -- VGA:
        R       : out   STD_LOGIC_VECTOR ( 3 downto 0);
        G       : out   STD_LOGIC_VECTOR ( 3 downto 0);
        -- Memory Bus:
        ADDR    : out   STD_LOGIC_VECTOR (23 downto 0);
        DATA    : inout STD_LOGIC_VECTOR (15 downto 0);
        OE      : out   STD_LOGIC := '1';
        WE      : out   STD_LOGIC := '1';
        MT_ADV  : out   STD_LOGIC := '0';
        MT_CLK  : out   STD_LOGIC := '0';
        MT_UB   : out   STD_LOGIC := '1';
        MT_LB   : out   STD_LOGIC := '1';
        MT_CE   : out   STD_LOGIC := '1';
        MT_CRE  : out   STD_LOGIC := '0';
        MT_WAIT : in    STD_LOGIC := '0';
        ST_STS  : in    STD_LOGIC := '0';
        RP      : out   STD_LOGIC := '1';
        ST_CE   : out   STD_LOGIC := '1'
    );
end TLC;

architecture Structural of TLC is

component DCM
    generic (CLKFX_MULTIPLY        : integer;
             CLKFX_DIVIDE          : integer);
    port    (CLKIN                 : in  std_logic;
             CLKFB                 : in  std_logic;
             DSSEN                 : in  std_logic;
             PSINCDEC              : in  std_logic;
             PSEN                  : in  std_logic;
             PSCLK                 : in  std_logic;
             RST                   : in  std_logic;
             CLK0                  : out std_logic;
             CLK90                 : out std_logic;
             CLK180                : out std_logic;
             CLK270                : out std_logic;
             CLK2X                 : out std_logic;
             CLK2X180              : out std_logic;
             CLKDV                 : out std_logic;
             CLKFX                 : out std_logic;
             CLKFX180              : out std_logic;
             LOCKED                : out std_logic;
             PSDONE                : out std_logic;
             STATUS                : out std_logic_vector(7 downto 0));
end component;

component BUFG
  port (I   : in  std_logic;
        O   : out std_logic);
end component;

signal CLK320MHz   : STD_LOGIC := '0';
signal CLK100MHz   : STD_LOGIC := '0';
signal CLK50MHz    : STD_LOGIC := '0';
signal CLK10MHz    : STD_LOGIC := '0';
signal CLK1MHz     : STD_LOGIC := '0';
signal CLK8KHz     : STD_LOGIC := '0';

signal counter1    : integer   := 0;
signal CLKFX1      : STD_LOGIC := '0';
signal CLKFX2      : STD_LOGIC := '0';
signal CLKFX3      : STD_LOGIC := '0';

signal GND         : STD_LOGIC := '0';

signal address     : integer   := 0;
signal wavdata     : integer   := 0;

signal pwm_counter : integer   := 0;
signal PWM         : STD_LOGIC := '0';

signal phase_accumulator : unsigned (31 downto 0) := (others => '0');

signal shift_ctr         : unsigned (4 downto 0) := (others => '0');
signal beep_counter      : unsigned (19 downto 0):= (others => '0');
signal message           : std_logic_vector(33 downto 0) :=
                           "1010100011101110111000101010000000";
begin

-- 50MHz clock
CLK50MHz <= CLK;

-- generate 100MHz clock
U0: DCM generic map (CLKFX_MULTIPLY  => 2,
                     CLKFX_DIVIDE    => 1)
        port    map (CLKIN           => CLK50MHz,
                     CLKFB           => GND,
                     DSSEN           => GND,
                     PSINCDEC        => GND,
                     PSEN            => GND,
                     PSCLK           => GND,
                     RST             => GND,
                     CLKFX           => CLKFX1);
U1: DCM generic map (CLKFX_MULTIPLY  => 2,
                     CLKFX_DIVIDE    => 10)
        port    map (CLKIN           => CLK50MHz,
                     CLKFB           => GND,
                     DSSEN           => GND,
                     PSINCDEC        => GND,
                     PSEN            => GND,
                     PSCLK           => GND,
                     RST             => GND,
                     CLKFX           => CLKFX2);
B0: BUFG port   map (CLKFX1, CLK100MHz);
B1: BUFG port   map (CLKFX2, CLK10MHz);

U2: DCM generic map (CLKFX_MULTIPLY  => 16,
                     CLKFX_DIVIDE    => 5)
        port    map (CLKIN           => CLK100MHz,
                     CLKFB           => GND,
                     DSSEN           => GND,
                     PSINCDEC        => GND,
                     PSEN            => GND,
                     PSCLK           => GND,
                     RST             => GND,
                     CLKFX           => CLKFX3);
B2: BUFG port   map (CLKFX3, CLK320MHz);

-- generate 8KHz clock
process (CLK100MHz)
begin
    if (CLK100MHz = '1' and CLK100MHz'event ) then
        if (counter1 = 12500/2-1) then
            counter1 <= 0;
            CLK8KHz <= not CLK8KHz;
        else
            counter1 <= counter1 + 1;
        end if;
    end if;
end process;


process(CLK320MHz)
begin
      if rising_edge(clk320MHz) then
         if beep_counter = x"FFFFF" then
            if shift_ctr = "00000" then
               message <= message(0) & message(33 downto 1);
            end if;
            shift_ctr <= shift_ctr + 1;
         end if;

         -- The constants are calculated as (desired freq)/320Mhz*2^32
--          if message(0) = '1' then
--            if beep_counter(19) = '1' then
--               phase_accumulator <= phase_accumulator + 1222387958;
--            else
--               phase_accumulator <= phase_accumulator + 1220374692;
--            end if;
--          else
--             phase_accumulator <= phase_accumulator + 1221381325;
--          end if;

         --if message(0) = '1' then
           --if beep_counter(19) = '1' then
              phase_accumulator <= phase_accumulator +
                                   1221381325 + wavdata*10000;
           --else
           --   phase_accumulator <= phase_accumulator + wavdata*4767088;
           --end if;
         --else
         --   phase_accumulator <= phase_accumulator + 1221381325;
         --end if;

         beep_counter <= beep_counter+1;
      end if;
end process;

-- output signal
--JA(0) <= (CLK100MHz and SW(0)) when PWM = '0' else (CLK10MHz and SW(0));

JA(0) <= std_logic(phase_accumulator(31));

-- apply PWM to music
process (CLK100MHz)
begin

    if (CLK100MHz = '1' and CLK100MHz'event ) then
        if (pwm_counter = 255) then
            pwm_counter <= 0;
        else
            pwm_counter <= pwm_counter+1;
        end if;
        if (pwm_counter <= wavdata) then
            PWM <= '1';
        else
            PWM <= '0';
        end if;
    end if;

end process;

-- play music from rom (8K rate)
process (CLK8KHz)
begin
    if (CLK8KHz = '1' and CLK8KHz'event and SW(1)='1') then
        address <= address + 1;
        if (address mod 2 = 0) then
            wavdata <= to_integer(unsigned(DATA(7 downto 0)));
        else
            wavdata <= to_integer(unsigned(DATA(15 downto 8)));
        end if;
    end if;
end process;

-- memory interface
OE      <= '0'; -- enable  read
WE      <= '1'; -- disable write
ADDR    <= std_logic_vector(to_unsigned(address, 24));
DATA    <= "ZZZZZZZZZZZZZZZZ";
MT_ADV  <= '0';
MT_CLK  <= '0';
MT_UB   <= '1';
MT_LB   <= '1';
MT_CE   <= '1'; -- disable ram
MT_CRE  <= '0';
RP      <= '1'; -- disable rom programming
ST_CE   <= '0'; -- enable rom

-- misc output
--LED <= DATA(15 downto 8);
--JA(7 downto 1) <= "0000000";
--R <= DATA(15 downto 12);
--G <= DATA(15 downto 12);

end Structural;
