#!/usr/bin/env python

opcodes = set()


class Opcode(object):
    def __init__(self, name, val, op_count, ticks, implementation, is_jump):
        self.name = name
        self.val = val
        self.op_count = op_count
        self.ticks = ticks
        self.implementation = implementation
        self.is_jump = is_jump

        opcodes.add(self)


####################################################################################################
# 8-BIT LOADS ######################################################################################
####################################################################################################


Opcode('NOP', '0x00', 0, 4, '', False)


def LDnn_n(nn, code):
    return Opcode('LD {}, n'.format(nn), code, 1, 8, 'cpu.reg.{} = cpu.current()[1];'.format(nn), False)


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
Opcode('LD A, (nn)', '0xFA', 2, 8, """u16 addr;
    std::memcpy(&addr, cpu.current(), 2);
    cpu.reg.A = cpu.bus.read(addr);""", False)

Opcode('LD (BC), A', '0x02', 0, 8,
       'cpu.bus.write(cpu.reg.getBC(), cpu.reg.A);', False)
Opcode('LD (DE), A', '0x12', 0, 8,
       'cpu.bus.write(cpu.reg.getDE(), cpu.reg.A);', False)
Opcode('LD (HL), A', '0x77', 0, 8,
       'cpu.bus.write(cpu.reg.getHL(), cpu.reg.A);', False)
Opcode('LD (nn), A', '0xEA', 2, 16, """u16 addr;
    std::memcpy(&addr, cpu.current(), 2);
    cpu.bus.write(addr, cpu.reg.A);""", False)

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


LDaddrHL_r2('B', '0x70')
LDaddrHL_r2('C', '0x71')
LDaddrHL_r2('D', '0x72')
LDaddrHL_r2('E', '0x73')
LDaddrHL_r2('H', '0x74')
LDaddrHL_r2('L', '0x75')
Opcode('LD (HL), n', '0x36', 1, 12,
       'cpu.bus.write(cpu.reg.getHL(), cpu.current()[1]);', False)


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
       'cpu.bus.write(0xFF00 + cpu.current()[1], cpu.reg.A);', False)
Opcode('LDH A, (n)', '0xF0', 1, 8,
       'cpu.reg.A = cpu.bus.read(0xFF00 + cpu.current()[1]);', False)

####################################################################################################
# 16-BIT LOADS #####################################################################################
####################################################################################################


def LDn_nn_16(r, code):
    return Opcode('LD {}, nn'.format(r), code, 2, 12, """u16 val;
    std::memcpy(&val, cpu.current() + 1, 2);
    cpu.reg.set{}(val);""".format(r), False)


LDn_nn_16('BC', '0x01')
LDn_nn_16('DE', '0x11')
LDn_nn_16('HL', '0x21')
LDn_nn_16('SP', '0x31')

Opcode('LD SP, HL', '0xF9', 0, 8, 'cpu.reg.setSP(cpu.reg.getHL());', False)
Opcode('LDHL SP, n', '0xF8', 1, 12, """i8 operand;
    std::memcpy(&operand, cpu.current() + 1, 1);
    const u16 addr = cpu.reg.getSP() + operand;
    cpu.reg.setHL(cpu.bus.read(addr));
    cpu.flags.resetZ();
    cpu.flags.resetN();
    // TODO more flags
""", False)

Opcode('LD (nn), SP', '0x08', 1, 20, """u16 addr;
    std::memcpy(&addr, cpu.current() + 1, 2);
    cpu.bus.write(addr, cpu.reg.getSP());""", False)


def PUSH_nn(r, code):
    return Opcode('PUSH {}'.format(r), code, 0, 16, """cpu.bus.write(cpu.reg.SP, cpu.reg.get{}());
    cpu.reg.SP -= 2;
    """.format(r), False)


PUSH_nn('AF', '0xF5')
PUSH_nn('BC', '0xC5')
PUSH_nn('DE', '0xD5')
PUSH_nn('HL', '0xE5')


def POP_nn(r, code):
    return Opcode('POP {}'.format(r), code, 0, 12, """const u8* const ptr = cpu.bus.ptr(cpu.reg.SP + 2);
    u16 val;
    std::memcpy(&val, ptr, 2);
    cpu.reg.set{}(val);
    cpu.reg.SP += 2;
    """.format(r), False)


POP_nn('AF', '0xF1')
POP_nn('BC', '0xC1')
POP_nn('DE', '0xD1')
POP_nn('HL', '0xE1')

####################################################################################################
# 8-BIT ALU ########################################################################################
####################################################################################################


def ADD(r, code):
    return Opcode('ADD A, {}'.format(r), code, 0, 4, 'alu::add(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


ADD('A', '0x87')
ADD('B', '0x80')
ADD('C', '0x81')
ADD('D', '0x82')
ADD('E', '0x83')
ADD('H', '0x84')
ADD('L', '0x85')
Opcode('ADD A, (HL)', '0x86', 0, 8,
       'alu::add(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def ADC(r, code):
    return Opcode('ADC A, {}'.format(r), code, 0, 4, 'alu::adc(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


ADC('A', '0x8F')
ADC('B', '0x88')
ADC('C', '0x89')
ADC('D', '0x8A')
ADC('E', '0x8B')
ADC('H', '0x8C')
ADC('L', '0x8D')
Opcode('ADC A, (HL)', '0x8E', 0, 8,
       'alu::adc(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def SUB(r, code):
    return Opcode('SUB A, {}'.format(r), code, 0, 4, 'alu::sub(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


SUB('A', '0x97')
SUB('B', '0x90')
SUB('C', '0x91')
SUB('D', '0x92')
SUB('E', '0x93')
SUB('H', '0x94')
SUB('L', '0x95')
Opcode('SUB A, (HL)', '0x96', 0, 8,
       'alu::sub(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


def SBC(r, code):
    return Opcode('SBC A, {}'.format(r), code, 0, 4, 'alu::sbc(cpu.reg.A, cpu.reg.{}, cpu);'.format(r), False)


SBC('A', '0x9F')
SBC('B', '0x98')
SBC('C', '0x99')
SBC('D', '0x9A')
SBC('E', '0x9B')
SBC('H', '0x9C')
SBC('L', '0x9D')
Opcode('SBC A, (HL)', '0x9E', 0, 8,
       'alu::sbc(cpu.reg.A, cpu.bus.read(cpu.reg.getHL()), cpu);', False)


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

# tools for adding new opcodes


def gen(s):
    ts = [s for s in s.split(' ') if s]
    ps = ts[1].split(',')
    return "ADD('{}', '0x{}', False)".format(ps[0], ts[2])


def gens(ss):
    print '\n'.join(gen(s) for s in ss.split('\n'))
