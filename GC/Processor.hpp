/*
 * Processor.cpp
 *
 */

#ifndef GC_PROCESSOR_HPP_
#define GC_PROCESSOR_HPP_

#include <GC/Processor.h>

#include <iostream>
#include <iomanip>
using namespace std;

#include "GC/Program.h"
#include "Access.h"
#include "Processor/FixInput.h"

#include "GC/Machine.hpp"
#include "Processor/ProcessorBase.hpp"

namespace GC
{

template <class T>
Processor<T>::Processor(Machine<T>& machine) :
		Processor<T>(machine, &machine)
{
}

template <class T>
Processor<T>::Processor(Memories<T>& memories, Machine<T>* machine) :
		machine(machine), memories(memories), PC(0), time(0),
		complexity(0)
{
}

template<class T>
Processor<T>::~Processor()
{
#ifdef VERBOSE
	cerr << "Finished after " << time << " instructions" << endl;
#endif
}

template <class T>
template <class U>
void Processor<T>::reset(const U& program, int arg)
{
    S.resize(program.num_reg(SBIT), "registers");
    C.resize(program.num_reg(CBIT), "registers");
    I.resize(program.num_reg(INT), "registers");
    set_arg(arg);
    PC = 0;
}

template <class T>
template <class U>
void Processor<T>::reset(const U& program)
{
    reset(program, 0);
    if (machine)
        machine->reset(program);
    memories.reset(program);
}

template<class T>
inline long long GC::Processor<T>::get_input(const int* params, bool interactive)
{
    assert(params[0] <= 64);
    return get_long_input(params, *this, interactive).get_si();
}

template<class T>
bigint GC::Processor<T>::get_long_input(const int* params,
        ProcessorBase& input_proc, bool interactive)
{
    bigint res = input_proc.get_input<FixInput_<bigint>>(interactive,
            &params[1]).items[0];
    int n_bits = *params;
    check_input(res, n_bits);
    return res;
}

template<class T>
void GC::Processor<T>::check_input(bigint in, int n_bits)
{
	auto test = in >> (n_bits - 1);
	if (n_bits == 1)
	{
		if (not (in == 0 or in == 1))
			throw runtime_error("input not a bit: " + in.get_str());
	}
	else if (not (test == 0 or test == -1))
	{
		throw runtime_error(
				"input too large for a " + std::to_string(n_bits)
						+ "-bit signed integer: " + in.get_str());
	}
}

template <class T>
void Processor<T>::bitdecc(const vector<int>& regs, const Clear& x)
{
    for (unsigned int i = 0; i < regs.size(); i++)
        C[regs[i]] = (x >> i) & 1;
}

template <class T>
void Processor<T>::bitdecint(const vector<int>& regs, const Integer& x)
{
    for (unsigned int i = 0; i < regs.size(); i++)
        I[regs[i]] = (x >> i) & 1;
}

template<class T>
template<class U>
void Processor<T>::load_dynamic_direct(const vector<int>& args,
        U& dynamic_memory)
{
    vector< ReadAccess<T> > accesses;
    if (args.size() % 3 != 0)
        throw runtime_error("invalid number of arguments");
    for (size_t i = 0; i < args.size(); i += 3)
        accesses.push_back({S[args[i]], args[i+1], args[i+2], complexity});
    T::load(accesses, dynamic_memory);
}

template<class T>
template<class U>
void GC::Processor<T>::load_dynamic_indirect(const vector<int>& args,
        U& dynamic_memory)
{
    vector< ReadAccess<T> > accesses;
    if (args.size() % 3 != 0)
        throw runtime_error("invalid number of arguments");
    for (size_t i = 0; i < args.size(); i += 3)
        accesses.push_back({S[args[i]], C[args[i+1]], args[i+2], complexity});
    T::load(accesses, dynamic_memory);
}

template<class T>
template<class U>
void GC::Processor<T>::store_dynamic_direct(const vector<int>& args,
        U& dynamic_memory)
{
    vector< WriteAccess<T> > accesses;
    if (args.size() % 2 != 0)
        throw runtime_error("invalid number of arguments");
    for (size_t i = 0; i < args.size(); i += 2)
        accesses.push_back({args[i+1], S[args[i]]});
    T::store(dynamic_memory, accesses);
    complexity += accesses.size() / 2 * T::default_length;
}

template<class T>
template<class U>
void GC::Processor<T>::store_dynamic_indirect(const vector<int>& args,
        U& dynamic_memory)
{
    vector< WriteAccess<T> > accesses;
    if (args.size() % 2 != 0)
        throw runtime_error("invalid number of arguments");
    for (size_t i = 0; i < args.size(); i += 2)
        accesses.push_back({C[args[i+1]], S[args[i]]});
    T::store(dynamic_memory, accesses);
    complexity += accesses.size() / 2 * T::default_length;
}

template<class T>
template<class U>
void GC::Processor<T>::store_clear_in_dynamic(const vector<int>& args,
        U& dynamic_memory)
{
    vector<ClearWriteAccess> accesses;
	check_args(args, 2);
    for (size_t i = 0; i < args.size(); i += 2)
    	accesses.push_back({C[args[i+1]], C[args[i]]});
    T::store_clear_in_dynamic(dynamic_memory, accesses);
}

template<class T>
template<class U>
void Processor<T>::mem_op(int n, Memory<U>& dest, const Memory<U>& source,
        Integer dest_address, Integer source_address)
{
    for (int i = 0; i < n; i++)
    {
        dest[dest_address + i] = source[source_address + i];
    }
}

template <class T>
void Processor<T>::xors(const vector<int>& args)
{
    check_args(args, 4);
    size_t n_args = args.size();
    for (size_t i = 0; i < n_args; i += 4)
    {
        S[args[i+1]].xor_(args[i], S[args[i+2]], S[args[i+3]]);
#ifndef FREE_XOR
        complexity += args[i];
#endif
    }
}

template<class T>
void Processor<T>::andm(const ::BaseInstruction& instruction)
{
    for (int i = 0; i < DIV_CEIL(instruction.get_n(), T::default_length); i++)
        S[instruction.get_r(0) + i] = S[instruction.get_r(1) + i]
                & C[instruction.get_r(2) + i];
}

template <class T>
void Processor<T>::and_(const vector<int>& args, bool repeat)
{
    check_args(args, 4);
    for (size_t i = 0; i < args.size(); i += 4)
    {
        S[args[i+1]].and_(args[i], S[args[i+2]], S[args[i+3]], repeat);
        complexity += args[i];
    }
}

template <class T>
void Processor<T>::input(const vector<int>& args)
{
    InputArgList a(args);
    for (auto x : a)
    {
        S[x.dest] = T::input(*this, x);
#ifdef DEBUG_INPUT
        cout << "input to " << args[i+2] << "/" << &S[args[i+2]] << endl;
#endif
    }
}

template <class T>
void Processor<T>::reveal(const vector<int>& args)
{
    for (size_t j = 0; j < args.size(); j += 3)
    {
        int n = args[j];
        int r0 = args[j + 1];
        int r1 = args[j + 2];
        if (n > max(T::default_length, Clear::N_BITS))
            assert(T::default_length == Clear::N_BITS);
        for (int i = 0; i < DIV_CEIL(n, T::default_length); i++)
            S[r1 + i].reveal(min(Clear::N_BITS, n - i * Clear::N_BITS),
                    C[r0 + i]);
    }
}

template <class T>
void Processor<T>::print_reg(int reg, int n, int size)
{
#ifdef DEBUG_VALUES
    cout << "print_reg " << typeid(T).name() << " " << reg << " " << &C[reg] << endl;
#endif
    bigint output;
    for (int i = 0; i < size; i++)
        output += bigint((unsigned long)C[reg + i].get()) << (T::default_length * i);
    T::out << "Reg[" << reg << "] = " << hex << showbase << output << dec << " # ";
    print_str(n);
    T::out << endl << flush;
}

template <class T>
void Processor<T>::print_reg_plain(Clear& value)
{
    T::out << hex << showbase << value << dec << flush;
}

template <class T>
void Processor<T>::print_reg_signed(unsigned n_bits, Integer reg)
{
    if (n_bits <= Clear::N_BITS)
    {
        auto value = C[reg];
        unsigned n_shift = 0;
        if (n_bits > 1)
            n_shift = sizeof(value.get()) * 8 - n_bits;
        if (n_shift > 63)
            n_shift = 0;
        T::out << dec << (value.get() << n_shift >> n_shift) << flush;
    }
    else
    {
        bigint tmp = 0;
        for (int i = 0; i < DIV_CEIL(n_bits, Clear::N_BITS); i++)
        {
            tmp += bigint((unsigned long)C[reg + i].get()) << (i * Clear::N_BITS);
        }
        if (tmp >= bigint(1) << (n_bits - 1))
            tmp -= bigint(1) << n_bits;
        T::out << dec << tmp << flush;
    }
}

template <class T>
void Processor<T>::print_chr(int n)
{
    T::out << (char)n << flush;
}

template <class T>
void Processor<T>::print_str(int n)
{
    T::out << string((char*)&n,sizeof(n)) << flush;
}

template <class T>
void Processor<T>::print_float(const vector<int>& args)
{
    T::out << bigint::get_float(C[args[0]], C[args[1]], C[args[2]], C[args[3]]) << flush;
}

template <class T>
void Processor<T>::print_float_prec(int n)
{
    T::out << setprecision(n);
}

} /* namespace GC */

#endif
