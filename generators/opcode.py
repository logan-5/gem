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


def LDr1_HL(r1, code):
    return Opcode('LD {}, (HL)'.format(r1), code, 0, 8, 'cpu.reg.{} = cpu.bus.read(cpu.reg.getHL());'.format(r1), False)


LDr1_HL('A', '0x7E')
LDr1_HL('B', '0x46')
LDr1_HL('C', '0x4E')
LDr1_HL('D', '0x56')
LDr1_HL('E', '0x5E')
LDr1_HL('H', '0x66')
LDr1_HL('L', '0x6E')


def LDHL_r2(r2, code):
    return Opcode('LD (HL), {}'.format(r2), code, 0, 8, 'cpu.bus.write(cpu.reg.getHL(), cpu.reg.{});'.format(r2), False)


LDHL_r2('B', '0x70')
LDHL_r2('C', '0x71')
LDHL_r2('D', '0x72')
LDHL_r2('E', '0x73')
LDHL_r2('H', '0x74')
LDHL_r2('L', '0x75')
Opcode('LD (HL), n', '0x36', 1, 12,
       'cpu.bus.write(cpu.reg.getHL(), cpu.current()[1]);', False)


# tools for adding new opcodes
def gen(s):
    ts = [s for s in s.split(' ') if s]
    ps = ts[1].split(',')
    return "LDr1_HL('{}', '0x{}', False)".format(ps[0], ts[2])


def gens(ss):
    print '\n'.join(gen(s) for s in ss.split('\n'))
