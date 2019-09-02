#!/usr/bin/env python

opcodes = set()

two_byte_prefixes = set()

helper_functions = []


class Opcode(object):
    def __init__(self, name, val, op_count, ticks, implementation, is_jump, second_byte=None):
        self.name = name
        self.val = val
        self.op_count = op_count
        self.ticks = ticks
        self.implementation = implementation
        self.is_jump = is_jump
        self.second_byte = second_byte

        opcodes.add(self)


####################################################################################################
# 8-BIT LOADS ######################################################################################
####################################################################################################


Opcode('NOP', '0x00', 0, 4, '', False)


def LDnn_n(nn, code):
    return Opcode('LD {}, n'.format(nn), code, 1, 8, 'cpu.reg.{} = cpu.readPC();'.format(nn), False)


LDnn_n('B', '0x06')
LDnn_n('C', '0x0E')
LDnn_n('D', '0x16')
LDnn_n('E', '0x1E')
LDnn_n('H', '0x26')
LDnn_n('L', '0x2E')


def LDr1_r2(r1, r2, code):
    return Opcode('LD {}, {}'.format(r1, r2), code, 0, 4, 'cpu.reg.{} = cpu.reg.{};'.format(r1, r2) if r1 != r2 else '', False)


LDr1_r2('A', 'A', '0x7F')
LDr1_r2('A', 'B', '0x78')
LDr1_r2('A', 'C', '0x79')
LDr1_r2('A', 'D', '0x7A')
LDr1_r2('A', 'E', '0x7B')
LDr1_r2('A', 'H', '0x7C')
LDr1_r2('A', 'L', '0x7D')

Opcode('LD A, (BC)', '0x0A', 0, 8,
       'cpu.reg.A = cpu.bus.read(cpu.reg.getBC());', False)
Opcode('LD A, (DE)', '0x1A', 0, 8,
       'cpu.reg.A = cpu.bus.read(cpu.reg.getDE());', False)
Opcode('LD A, (nn)', '0xFA', 2, 8,
       'cpu.reg.A = cpu.bus.read(cpu.readPC16());', False)
Opcode('LD A, n', '0x3E', 1, 8, 'cpu.reg.A = cpu.readPC();', False)

Opcode('LD (BC), A', '0x02', 0, 8,
       'cpu.bus.write(cpu.reg.getBC(), cpu.reg.A);', False)
Opcode('LD (DE), A', '0x12', 0, 8,
       'cpu.bus.write(cpu.reg.getDE(), cpu.reg.A);', False)
Opcode('LD (HL), A', '0x77', 0, 8,
       'cpu.bus.write(cpu.reg.getHL(), cpu.reg.A);', False)
Opcode('LD (nn), A', '0xEA', 2, 16,
       'cpu.bus.write(cpu.readPC16(), cpu.reg.A);', False)

LDr1_r2('B', 'B', '0x40')
LDr1_r2('B', 'C', '0x41')
LDr1_r2('B', 'D', '0x42')
LDr1_r2('B', 'E', '0x43')
LDr1_r2('B', 'H', '0x44')
LDr1_r2('B', 'L', '0x45')

LDr1_r2('C', 'B', '0x48')
LDr1_r2('C', 'C', '0x49')
LDr1_r2('C', 'D', '0x4A')
LDr1_r2('C', 'E', '0x4B')
LDr1_r2('C', 'H', '0x4C')
LDr1_r2('C', 'L', '0x4D')

LDr1_r2('D', 'B', '0x50')
LDr1_r2('D', 'C', '0x51')
LDr1_r2('D', 'D', '0x52')
LDr1_r2('D', 'E', '0x53')
LDr1_r2('D', 'H', '0x54')
LDr1_r2('D', 'L', '0x55')

LDr1_r2('E', 'B', '0x58')
LDr1_r2('E', 'C', '0x59')
LDr1_r2('E', 'D', '0x5A')
LDr1_r2('E', 'E', '0x5B')
LDr1_r2('E', 'H', '0x5C')
LDr1_r2('E', 'L', '0x5D')

LDr1_r2('H', 'B', '0x60')
LDr1_r2('H', 'C', '0x61')
LDr1_r2('H', 'D', '0x62')
LDr1_r2('H', 'E', '0x63')
LDr1_r2('H', 'H', '0x64')
LDr1_r2('H', 'L', '0x65')

LDr1_r2('L', 'B', '0x68')
LDr1_r2('L', 'C', '0x69')
LDr1_r2('L', 'D', '0x6A')
LDr1_r2('L', 'E', '0x6B')
LDr1_r2('L', 'H', '0x6C')
LDr1_r2('L', 'L', '0x6D')


def LDr1_addrHL(r1, code):
    return Opcode('LD {}, (HL)'.format(r1), code, 0, 8, 'cpu.reg.{} = cpu.bus.read(cpu.reg.getHL());'.format(r1), False)


LDr1_addrHL('A', '0x7E')
LDr1_addrHL('B', '0x46')
LDr1_addrHL('C', '0x4E')
LDr1_addrHL('D', '0x56')
LDr1_addrHL('E', '0x5E')
LDr1_addrHL('H', '0x66')
LDr1_addrHL('L', '0x6E')


def LDaddrHL_r2(r2, code):
    return Opcode('LD (HL), {}'.format(r2), code, 0, 8, 'cpu.bus.write(cpu.reg.getHL(), cpu.reg.{});'.format(r2), False)


def LDn_A(n, code):
    return Opcode('LD {}, A'.format(n), code, 0, 4, 'cpu.reg.{} = cpu.reg.A;'.format(n), False)


LDn_A('B', '0x47')
LDn_A('C', '0x4F')
LDn_A('D', '0x57')
LDn_A('E', '0x5F')
LDn_A('H', '0x67')
LDn_A('L', '0x6F')


LDaddrHL_r2('B', '0x70')
LDaddrHL_r2('C', '0x71')
LDaddrHL_r2('D', '0x72')
LDaddrHL_r2('E', '0x73')
LDaddrHL_r2('H', '0x74')
LDaddrHL_r2('L', '0x75')
Opcode('LD (HL), n', '0x36', 1, 12,
       'cpu.bus.write(cpu.reg.getHL(), cpu.readPC());', False)


Opcode('LD A, (C)', '0xF2', 0, 8,
       'cpu.reg.A = cpu.bus.read(0xFF00 + cpu.reg.C);', False)
Opcode('LD (C), A', '0xE2', 0, 8,
       'cpu.bus.write(0xFF00 + cpu.reg.C, cpu.reg.A);', False)

Opcode('LDD A, (HL)', '0x3A', 0, 8, """cpu.reg.A = cpu.bus.read(cpu.reg.getHL());
    cpu.reg.decHL();""", False)
Opcode('LDD (HL), A', '0x32', 0, 8, """cpu.bus.write(cpu.reg.getHL(), cpu.reg.A);
    cpu.reg.decHL();""", False)
Opcode('LDI A, (HL)', '0x2A', 0, 8, """cpu.reg.A = cpu.bus.read(cpu.reg.getHL());
    cpu.reg.incHL();""", False)
Opcode('LDI (HL), A', '0x22', 0, 8, """cpu.bus.write(cpu.reg.getHL(), cpu.reg.A);
    cpu.reg.incHL();""", False)
Opcode('LDH (n), A', '0xE0', 1, 8,
       'cpu.bus.write(0xFF00 + cpu.readPC(), cpu.reg.A);', False)
Opcode('LDH A, (n)', '0xF0', 1, 8,
       'cpu.reg.A = cpu.bus.read(0xFF00 + cpu.readPC());', False)

####################################################################################################
# 16-BIT LOADS #####################################################################################
####################################################################################################


def LDn_nn_16(r, code):
    return Opcode('LD {}, nn'.format(r), code, 2, 12, 'cpu.reg.set{}(cpu.readPC16());'.format(r), False)


LDn_nn_16('BC', '0x01')
LDn_nn_16('DE', '0x11')
LDn_nn_16('HL', '0x21')
LDn_nn_16('SP', '0x31')

Opcode('LD SP, HL', '0xF9', 0, 8, 'cpu.reg.setSP(cpu.reg.getHL());', False)
Opcode('LDHL SP, n', '0xF8', 1, 12, """const u16 addr = alu::add16Signed8(cpu.reg.SP, cpu.readPC(), cpu);
    cpu.reg.setHL(cpu.bus.read(addr));""", False)

Opcode('LD (nn), SP', '0x08', 1, 20,
       'cpu.bus.write(cpu.readPC16(), cpu.reg.getSP());', False)

helper_functions.append("""\
void pushStack(gem::u16 val, gem::CPU& cpu) {
    cpu.reg.SP -= 2;
    cpu.bus.write(cpu.reg.SP, val);
}""")
helper_functions.append("""\
gem::u16 popStack(gem::CPU& cpu) {
    using namespace gem;
    const u8* const ptr = cpu.bus.ptr(cpu.reg.SP);
    u16 val;
    std::memcpy(&val, ptr, sizeof val);
    cpu.reg.SP += 2;
    return val;
}""")


def PUSH_nn(r, code):
    return Opcode('PUSH {}'.format(r), code, 0, 16, 'pushStack(cpu.reg.get{}(), cpu);'.format(r), False)


PUSH_nn('AF', '0xF5')
PUSH_nn('BC', '0xC5')
PUSH_nn('DE', '0xD5')
PUSH_nn('HL', '0xE5')


def POP_nn(r, code):
    return Opcode('POP {}'.format(r), code, 0, 12, 'cpu.reg.set{}(popStack(cpu));'.format(r), False)


POP_nn('AF', '0xF1')
POP_nn('BC', '0xC1')
POP_nn('DE', '0xD1')
POP_nn('HL', '0xE1')

####################################################################################################
# 8-BIT ALU ########################################################################################
####################################################################################################


def ADD(r, code):
    return Opcode('ADD A, {}'.format(r), code, 0, 4, 'alu::add8(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


ADD('A', '0x87')
ADD('B', '0x80')
ADD('C', '0x81')
ADD('D', '0x82')
ADD('E', '0x83')
ADD('H', '0x84')
ADD('L', '0x85')
Opcode('ADD A, (HL)', '0x86', 0, 8,
       'alu::add8(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def ADC(r, code):
    return Opcode('ADC A, {}'.format(r), code, 0, 4, 'alu::adc8(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


ADC('A', '0x8F')
ADC('B', '0x88')
ADC('C', '0x89')
ADC('D', '0x8A')
ADC('E', '0x8B')
ADC('H', '0x8C')
ADC('L', '0x8D')
Opcode('ADC A, (HL)', '0x8E', 0, 8,
       'alu::adc8(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def SUB(r, code):
    return Opcode('SUB A, {}'.format(r), code, 0, 4, 'alu::sub8(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


SUB('A', '0x97')
SUB('B', '0x90')
SUB('C', '0x91')
SUB('D', '0x92')
SUB('E', '0x93')
SUB('H', '0x94')
SUB('L', '0x95')
Opcode('SUB A, (HL)', '0x96', 0, 8,
       'alu::sub8(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def SBC(r, code):
    return Opcode('SBC A, {}'.format(r), code, 0, 4, 'alu::sbc8(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


SBC('A', '0x9F')
SBC('B', '0x98')
SBC('C', '0x99')
SBC('D', '0x9A')
SBC('E', '0x9B')
SBC('H', '0x9C')
SBC('L', '0x9D')
Opcode('SBC A, (HL)', '0x9E', 0, 8,
       'alu::sbc8(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def AND(r, code):
    return Opcode('AND {}'.format(r), code, 0, 4, 'alu::and_(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


AND('A', '0xA7')
AND('B', '0xA0')
AND('C', '0xA1')
AND('D', '0xA2')
AND('E', '0xA3')
AND('H', '0xA4')
AND('L', '0xA5')
Opcode('AND (HL)', '0xA6', 0, 8,
       'alu::and_(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def OR(r, code):
    return Opcode('OR {}'.format(r), code, 0, 4, 'alu::or_(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


OR('A', '0xB7')
OR('B', '0xB0')
OR('C', '0xB1')
OR('D', '0xB2')
OR('E', '0xB3')
OR('H', '0xB4')
OR('L', '0xB5')
Opcode('OR (HL)', '0xB6', 0, 8,
       'alu::or_(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def XOR(r, code):
    return Opcode('XOR {}'.format(r), code, 0, 4, 'alu::xor_(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


XOR('A', '0xAF')
XOR('B', '0xA8')
XOR('C', '0xA9')
XOR('D', '0xAA')
XOR('E', '0xAB')
XOR('H', '0xAC')
XOR('L', '0xAD')
Opcode('XOR (HL)', '0xAE', 0, 8,
       'alu::xor_(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def CP(r, code):
    return Opcode('CP {}'.format(r), code, 0, 4, 'alu::cp(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


CP('A', '0xBF')
CP('B', '0xB8')
CP('C', '0xB9')
CP('D', '0xBA')
CP('E', '0xBB')
CP('H', '0xBC')
CP('L', '0xBD')
Opcode('CP (HL)', '0xBE', 0, 8,
       'alu::cp(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)
Opcode('CP n', '0xFE', 1, 8,
       'alu::cp(cpu.reg.A, cpu.readPC(), cpu);', False)


def INC(r, code):
    return Opcode('INC {}'.format(r), code, 0, 4, 'alu::inc(cpu.reg.{}, cpu);'.format(r), False)


INC('A', '0x3C')
INC('B', '0x04')
INC('C', '0x0C')
INC('D', '0x14')
INC('E', '0x1C')
INC('H', '0x24')
INC('L', '0x2C')
Opcode('INC (HL)', '0x34', 0, 12, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
        alu::inc(tmp, cpu);
        cpu.bus.write(cpu.reg.getHL(), tmp);""", False)


def DEC(r, code):
    return Opcode('DEC {}'.format(r), code, 0, 4, 'alu::dec(cpu.reg.{}, cpu);'.format(r), False)


DEC('A', '0x3D')
DEC('B', '0x05')
DEC('C', '0x0D')
DEC('D', '0x15')
DEC('E', '0x1D')
DEC('H', '0x25')
DEC('L', '0x2D')
Opcode('DEC (HL)', '0x35', 0, 12, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
        alu::dec(tmp, cpu);
        cpu.bus.write(cpu.reg.getHL(), tmp);""", False)

####################################################################################################
# 16-BIT ARITHMETIC ################################################################################
####################################################################################################


def ADD_HL(n, code):
    return Opcode('ADD HL, {}'.format(n), code, 0, 8, 'cpu.reg.setHL(alu::add16(cpu.reg.getHL(), cpu.reg.get{}(), cpu));'.format(n), False)


ADD_HL('BC', '0x09')
ADD_HL('DE', '0x19')
ADD_HL('HL', '0x29')
ADD_HL('SP', '0x39')

Opcode('ADD SP, n', '0xE8', 2, 16,
       'cpu.reg.SP = alu::add16Signed8(cpu.reg.SP, cpu.readPC(), cpu);', False)


def INC_NN(nn, code):
    return Opcode('INC {}'.format(nn), code, 0, 8, 'cpu.reg.inc{}();'.format(nn), False)


INC_NN('BC', '0x03')
INC_NN('DE', '0x13')
INC_NN('HL', '0x23')
INC_NN('SP', '0x33')


def DEC_NN(nn, code):
    return Opcode('DEC {}'.format(nn), code, 0, 8, 'cpu.reg.dec{}();'.format(nn), False)


DEC_NN('BC', '0x0B')
DEC_NN('DE', '0x1B')
DEC_NN('HL', '0x2B')
DEC_NN('SP', '0x3B')

####################################################################################################
# MISCELLANEOUS ####################################################################################
####################################################################################################


two_byte_prefixes.add('0xCB')


def SWAP(n, code):
    return Opcode('SWAP {}'.format(n), '0xCB', 0, 8, 'alu::swapNybbles(cpu.reg.{}, cpu);'.format(n), False, code)


SWAP('A', '0x37')
SWAP('B', '0x30')
SWAP('C', '0x31')
SWAP('D', '0x32')
SWAP('E', '0x33')
SWAP('H', '0x34')
SWAP('L', '0x35')
Opcode('SWAP (HL)', '0xCB', 0, 8, """u8 temp = cpu.bus.read(cpu.reg.getHL());
        alu::swapNybbles(temp, cpu);
        cpu.bus.write(cpu.reg.getHL(), temp);""", False, '0x36')

Opcode('DAA', '0x27', 0, 4, 'alu::decimalAdjust(cpu.reg.A, cpu);', False)

Opcode('CPL', '0x2F', 0, 4, 'alu::complement(cpu.reg.A, cpu);', False)

Opcode('CCF', '0x3F', 0, 4, """cpu.flags.resetN();
        cpu.flags.resetH();
        cpu.flags.toggleC();""", False)

Opcode('SCF', '0x37', 0, 4, """cpu.flags.resetN();
        cpu.flags.resetH();
        cpu.flags.setC();""", False)

####################################################################################################
# ROTATES & SHIFTS #################################################################################
####################################################################################################

Opcode('RLCA', '0x07', 0, 4, 'alu::rlc(cpu.reg.A, cpu);', False)
Opcode('RLA', '0x17', 0, 4, 'alu::rl(cpu.reg.A, cpu);', False)


def RLC_n(n, code):
    return Opcode('RLC {}'.format(n), '0xCB', 0, 8, 'alu::rlc(cpu.reg.{}, cpu);'.format(n), False, code)


RLC_n('A', '0x07')
RLC_n('B', '0x00')
RLC_n('C', '0x01')
RLC_n('D', '0x02')
RLC_n('E', '0x03')
RLC_n('H', '0x04')
RLC_n('L', '0x05')
Opcode('RLC (HL)', '0xCB', 0, 16, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
    alu::rlc(tmp, cpu);
    cpu.bus.write(cpu.reg.getHL(), tmp);""", False, '0x06')


def RL_n(n, code):
    return Opcode('RL {}'.format(n), '0xCB', 0, 8, 'alu::rl(cpu.reg.{}, cpu);'.format(n), False, code)


RL_n('A', '0x17')
RL_n('B', '0x10')
RL_n('C', '0x11')
RL_n('D', '0x12')
RL_n('E', '0x13')
RL_n('H', '0x14')
RL_n('L', '0x15')
Opcode('RL (HL)', '0xCB', 0, 16, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
    alu::rl(tmp, cpu);
    cpu.bus.write(cpu.reg.getHL(), tmp);""", False, '0x16')


def RRC_n(n, code):
    return Opcode('RRC {}'.format(n), '0xCB', 0, 8, 'alu::rrc(cpu.reg.{}, cpu);'.format(n), False, code)


RRC_n('A', '0x0F')
RRC_n('B', '0x08')
RRC_n('C', '0x09')
RRC_n('D', '0x0A')
RRC_n('E', '0x0B')
RRC_n('H', '0x0C')
RRC_n('L', '0x0D')
Opcode('RRC (HL)', '0xCB', 0, 16, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
    alu::rrc(tmp, cpu);
    cpu.bus.write(cpu.reg.getHL(), tmp);""", False, '0x0E')


def RR_n(n, code):
    return Opcode('RR {}'.format(n), '0xCB', 0, 8, 'alu::rr(cpu.reg.{}, cpu);'.format(n), False, code)


RR_n('A', '0x1F')
RR_n('B', '0x18')
RR_n('C', '0x19')
RR_n('D', '0x1A')
RR_n('E', '0x1B')
RR_n('H', '0x1C')
RR_n('L', '0x1D')
Opcode('RR (HL)', '0xCB', 0, 16, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
    alu::rr(tmp, cpu);
    cpu.bus.write(cpu.reg.getHL(), tmp);""", False, '0x1E')


def SLA_n(n, code):
    return Opcode('SLA {}'.format(n), '0xCB', 0, 8, 'alu::sla(cpu.reg.{}, cpu);'.format(n), False, code)


SLA_n('A', '0x27')
SLA_n('B', '0x20')
SLA_n('C', '0x21')
SLA_n('D', '0x22')
SLA_n('E', '0x23')
SLA_n('H', '0x24')
SLA_n('L', '0x25')
Opcode('SLA (HL)', '0xCB', 0, 16, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
    alu::sla(tmp, cpu);
    cpu.bus.write(cpu.reg.getHL(), tmp);""", False, '0x26')


def SRA_n(n, code):
    return Opcode('SRA {}'.format(n), '0xCB', 0, 8, 'alu::sra(cpu.reg.{}, cpu);'.format(n), False, code)


SRA_n('A', '0x2F')
SRA_n('B', '0x28')
SRA_n('C', '0x29')
SRA_n('D', '0x2A')
SRA_n('E', '0x2B')
SRA_n('H', '0x2C')
SRA_n('L', '0x2D')
Opcode('SRA (HL)', '0xCB', 0, 16, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
    alu::sra(tmp, cpu);
    cpu.bus.write(cpu.reg.getHL(), tmp);""", False, '0x2E')


def SRL_n(n, code):
    return Opcode('SRL {}'.format(n), '0xCB', 0, 8, 'alu::srl(cpu.reg.{}, cpu);'.format(n), False, code)


SRL_n('A', '0x3F')
SRL_n('B', '0x38')
SRL_n('C', '0x39')
SRL_n('D', '0x3A')
SRL_n('E', '0x3B')
SRL_n('H', '0x3C')
SRL_n('L', '0x3D')
Opcode('SRL (HL)', '0xCB', 0, 16, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
    alu::srl(tmp, cpu);
    cpu.bus.write(cpu.reg.getHL(), tmp);""", False, '0x3E')

####################################################################################################
# BIT OPCODES ######################################################################################
####################################################################################################


def BIT(n, code):
    for b in xrange(0, 8):
        code_i = int(code, base=0) + b * 8
        Opcode('BIT {1}, {0}'.format(n, b), '0xCB', 0, 8,
               'alu::bit<{1}>(cpu.reg.{0}, cpu);'.format(n, b), False, hex(code_i))


BIT('A', '0x47')
BIT('B', '0x40')
BIT('C', '0x41')
BIT('D', '0x42')
BIT('E', '0x43')
BIT('H', '0x44')
BIT('L', '0x45')
for b in xrange(0, 8):
    code_i = int('0x46', base=0) + b * 8
    Opcode('BIT {}, (HL)'.format(b), '0xCB', 0, 16,
           'alu::bit<{}>(cpu.bus.read(cpu.reg.getHL()), cpu);'.format(b), False, hex(code_i))


def SET(n, code):
    for b in xrange(0, 8):
        code_i = int(code, base=0) + b * 8
        Opcode('SET {1}, {0}'.format(n, b), '0xCB', 0, 8,
               'alu::set<{1}>(cpu.reg.{0}, cpu);'.format(n, b), False, hex(code_i))


SET('A', '0xC7')
SET('B', '0xC0')
SET('C', '0xC1')
SET('D', '0xC2')
SET('E', '0xC3')
SET('H', '0xC4')
SET('L', '0xC5')
for b in xrange(0, 8):
    code_i = int('0xC6', base=0) + b * 8
    Opcode('SET {}, (HL)'.format(b), '0xCB', 0, 16, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
        alu::set<{}>(tmp, cpu);
        cpu.bus.write(cpu.reg.getHL(), tmp);""".format(b), False, hex(code_i))


def RES(n, code):
    for b in xrange(0, 8):
        code_i = int(code, base=0) + b * 8
    Opcode('RES {1}, {0}'.format(n, b), '0xCB', 0, 8,
           'alu::res<{1}>(cpu.reg.{0}, cpu);'.format(n, b), False, hex(code_i))


RES('A', '0x87')
RES('B', '0x80')
RES('C', '0x81')
RES('D', '0x82')
RES('E', '0x83')
RES('H', '0x84')
RES('L', '0x85')
for b in xrange(0, 8):
    code_i = int('0x86', base=0) + b * 8
    Opcode('RES {}, (HL)'.format(b), '0xCB', 0, 16, """u8 tmp = cpu.bus.read(cpu.reg.getHL());
        alu::res<{}>(tmp, cpu);
        cpu.bus.write(cpu.reg.getHL(), tmp);""".format(b), False, hex(code_i))

####################################################################################################
# JUMPS ############################################################################################
####################################################################################################

helper_functions.append("""\
void JP_nn_impl(gem::CPU& cpu) {
    cpu.reg.PC = cpu.readPC16();
}""")
Opcode('JP nn', '0xC3', 2, 12, 'JP_nn_impl(cpu);', True)


def JP_cc(cc, code, flag, reset):
    return Opcode('JP {}, nn'.format(cc), code, 2, 12, """if ({1}cpu.flags.get{0}()) {{
        JP_nn_impl(cpu);
        cpu.ticks += 4;
    }} else {{ (void)cpu.readPC16(); }}""".format(flag, '!' if reset else ''), True)


JP_cc('NZ', '0xC2', 'Z', True)
JP_cc('Z', '0xCA', 'Z', False)
JP_cc('NC', '0xD2', 'C', True)
JP_cc('C', '0xDA', 'C', False)

Opcode('JP (HL)', '0xE9', 0, 4, 'cpu.reg.PC = cpu.bus.read(cpu.reg.getHL());', True)

helper_functions.append("""\
void JR_n_impl(gem::CPU& cpu) {
    using namespace gem;
    const u8 byte = cpu.readPC();
    i8 val;
    std::memcpy(&val, &byte, sizeof val);
    cpu.reg.PC += val;
}""")

Opcode('JR n', '0x18', 1, 12, 'JR_n_impl(cpu);', True)


def JR_cc_n(cc, code, flag, reset):
    return Opcode('JR {}, n'.format(cc), code, 1, 8, """if ({1}cpu.flags.get{0}()) {{
        JR_n_impl(cpu);
        cpu.ticks += 4;
    }} else {{ (void)cpu.readPC(); }}""".format(flag, '!' if reset else ''), True)


JR_cc_n('NZ', '0x20', 'Z', True)
JR_cc_n('Z', '0x28', 'Z', False)
JR_cc_n('NC', '0x30', 'C', True)
JR_cc_n('C', '0x38', 'C', False)

helper_functions.append("""\
void call_impl(gem::CPU& cpu) {
    using namespace gem;
    const u16 target = cpu.readPC16();
    pushStack(cpu.reg.PC, cpu);
    cpu.reg.PC = target;
}""")

Opcode('CALL nn', '0xCD', 2, 24, 'call_impl(cpu);', True)


def CALL_cc_nn(cc, code, flag, reset):
    return Opcode('CALL {}, nn'.format(cc), code, 2, 12, """if ({1}cpu.flags.get{0}()) {{
        call_impl(cpu);
        cpu.ticks += 12;
    }}""".format(flag, '!' if reset else ''), True)


CALL_cc_nn('NZ', '0xC4', 'Z', True)
CALL_cc_nn('Z', '0xCC', 'Z', False)
CALL_cc_nn('NC', '0xD4', 'C', True)
CALL_cc_nn('C', '0xDC', 'C', False)

####################################################################################################
# RESTARTS #########################################################################################
####################################################################################################

for n in (0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38):
    code = hex(n + 0xC7)
    Opcode('RST {}'.format(hex(n)), code, 0, 32, """pushStack(cpu.reg.PC, cpu);
    cpu.reg.PC = 0x00 + {};
    """.format(hex(n)), True)

####################################################################################################
# RETURNS ##########################################################################################
####################################################################################################

helper_functions.append("""\
void ret_impl(gem::CPU& cpu) {
    using namespace gem;
    const u16 next = popStack(cpu);
    cpu.reg.PC = next;   
}""")

Opcode('RET', '0xC9', 0, 8, 'ret_impl(cpu);', False)


def RET_cc(cc, code, flag, reset):
    return Opcode('RET {}'.format(cc), code, 0, 8, """if ({1}cpu.flags.get{0}()) {{
        ret_impl(cpu);
        cpu.ticks += 12;
    }}""".format(flag, '!' if reset else ''), True)


RET_cc('NZ', '0xC0', 'Z', True)
RET_cc('Z', '0xC8', 'Z', False)
RET_cc('NC', '0xD0', 'C', True)
RET_cc('C', '0xD8', 'C', False)

# tools for adding new opcodes


def gen(s):
    ts = [s for s in s.split(' ') if s]
    ps = ts[1].split(',')
    return "ADD('{}', '0x{}', False)".format(ps[0], ts[2])


def gens(ss):
    print '\n'.join(gen(s) for s in ss.split('\n'))
