//
// Created by Roj234 on 2024/9/25 0025.
//

#include <cstdint>
#include "_Roj234_FastJNI.hpp"

// 我也不知道编译的时候有没有strlen，就这样吧
bool checkIsFastJni(ciMethod *method) {
    const char *name = method->name()->as_utf8();
    size_t strlen_ = strlen(name);
    if (strlen_ > 7 &&
        name[strlen_-1] == 'I' && name[strlen_-2] == 'N' && name[strlen_-3] == 'J' &&
        name[strlen_-4] == 't' && name[strlen_-5] == 's' && name[strlen_-6] == 'a' && name[strlen_-7] == 'F') {
        return true;
    }

    return false;
}

int32_t FastJNICALL myMethod(int32_t arg1) {
    return arg1 * 233;
}

#include "stubGenerator_x86_64.hpp"
#define __ _masm->
address AsmLinker1() {
    __ align(CodeEntryAlignment);
    //StubCodeMark mark(this, "StubRoutines", "generate_MyMethod");
    address start = __ pc();

    const Register ra0 = c_rarg0;
    const Register ra1 = c_rarg1;
    const Register ra2 = c_rarg2;
    const Register ra3 = c_rarg3;

    __ enter();
    myMethod(ra0);
    //__ mov(rax, 0xabcdef);
    __ leave(); // required for proper stackwalking of RuntimeStub frame
    __ ret(0);
    return start;
}

void LinkFastJni(LibraryCallKit *kit, const char *stubName, const char *stubDesc, uint8_t stubDescLen, address stubAddr);

#include <stdlib.h>
JVMState* LibraryIntrinsic::generate(JVMState* jvms) {
    LibraryCallKit kit(jvms, this);
    ciMethod* callee = kit.callee();
    // Try to inline the intrinsic.
    if (checkIsFastJni(callee)) {
        const char *owner, *name, *desc;
        owner = callee->holder()->name()->as_utf8();
        name = callee->name()->as_utf8();
        desc = callee->signature()->as_symbol()->as_utf8();

        char *stubName = malloc(strlen(owner)+strlen(name)+1);
        assert(stubName != nullptr, "FastJNI OOM");
        //char stubName[256];
        //assert(strlen(owner)+strlen(name) < 255, "FastJNI String Buffer Overflow");
        size_t len = strlen(owner);
        strncpy(stubName, owner, len);
        stubName[len] = '.';
        strncpy(stubName+len+1, name, strlen(name));

        uint8_t argSize = 0;
        uint8_t argCount = 0;
        char argType[256];
        len = strlen(desc);
        size_t j = 0;
        for (size_t i = 0; i < len; ++i) {
            if (desc[i] == '(' || desc[i] == ')') continue;
            argType[j++] = desc[i];
            argCount++;
            if (desc[i] == 'J' || desc[i] == 'D') argSize += 2;
            else argSize += 1;
            if (desc[i] == 'L') {
                while (desc[i++] != ';');
            }
        }
        argType[j] = 0;

        char *p = getenv("_VAR1");
        if (p)
            tty->print_cr("FastJNI VAR %s", p);
        LinkFastJni(&kit, stubName, argType, argCount-1, reinterpret_cast<address>(p ? &myMethod : AsmLinker1()));
        kit.push_result();
        tty->print_cr("FastJNI end");
        return kit.transfer_exceptions_into_jvms();
    }

    return this->_generate(jvms);
}

void LinkFastJni(LibraryCallKit *kit, const char *stubName, const char *stubDesc, uint8_t stubDescLen, address stubAddr) {
    const TypeFunc *_jni_signature;

#define AAAA \
	switch (stubDesc[i]) { \
        case 'L': jType = TypePtr::BOTTOM;/* Is BUTTOM or NOTNULL? */ break; \
		case 'I': case 'F': jType = TypeInt::INT; break; \
		case 'J': case 'D': jType = TypeLong::LONG; break; \
 \
		case 'Z': jType = TypeInt::BOOL; break; \
		case 'B': jType = TypeInt::BYTE; break; \
		case 'S': jType = TypeInt::SHORT; break; \
		case 'C': jType = TypeInt::CHAR; break; \
	}

    // 生成参数类型
    const Type** fields = TypeTuple::fields(stubDescLen);
    uint8_t i=0;
    const Type *jType;

    int argp = TypeFunc::Parms;
    for (; i < stubDescLen; i++) {
        AAAA
        fields[argp++] = jType;
    }
    const TypeTuple* domain = TypeTuple::make(TypeFunc::Parms+stubDescLen, fields);

    fields = TypeTuple::fields(1);
    AAAA
    fields[TypeFunc::Parms+0] = jType; // void
    const TypeTuple* range = TypeTuple::make(TypeFunc::Parms, fields);

    _jni_signature = TypeFunc::make(domain, range);

#undef AAAA

    int _jni_flag = LibraryCallKit::RC_LEAF | LibraryCallKit::RC_NO_FP;
    tty->print_cr("FastJNI Name %s Flag %d Stub Pointer %d Argc %d", stubName, _jni_flag, stubAddr, stubDescLen);
    switch (stubDescLen) {
        case 0:
            kit -> make_runtime_call(_jni_flag, _jni_signature,
                                     stubAddr, stubName, TypePtr::BOTTOM);
            break;
        case 1:
            kit -> make_runtime_call(_jni_flag, _jni_signature,
                                     stubAddr, stubName, TypePtr::BOTTOM,
                                     kit -> argument(0));
            break;
        case 2:
            kit -> make_runtime_call(_jni_flag, _jni_signature,
                                     stubAddr, stubName, TypePtr::BOTTOM,
                                     kit -> argument(0), kit -> argument(1));
            break;
        case 3:
            kit -> make_runtime_call(_jni_flag, _jni_signature,
                                     stubAddr, stubName, TypePtr::BOTTOM,
                                     kit -> argument(0), kit -> argument(1), kit -> argument(2));
            break;
        case 4:
            kit -> make_runtime_call(_jni_flag, _jni_signature,
                                     stubAddr, stubName, TypePtr::BOTTOM,
                                     kit -> argument(0), kit -> argument(1), kit -> argument(2),
                                     kit -> argument(3));
            break;
        case 5:
            kit -> make_runtime_call(_jni_flag, _jni_signature,
                                     stubAddr, stubName, TypePtr::BOTTOM,
                                     kit -> argument(0), kit -> argument(1), kit -> argument(2),
                                     kit -> argument(3), kit -> argument(4));
            break;
        case 6:
            kit -> make_runtime_call(_jni_flag, _jni_signature,
                                     stubAddr, stubName, TypePtr::BOTTOM,
                                     kit -> argument(0), kit -> argument(1), kit -> argument(2),
                                     kit -> argument(3), kit -> argument(4), kit -> argument(5));
            break;
        case 7:
            kit -> make_runtime_call(_jni_flag, _jni_signature,
                                     stubAddr, stubName, TypePtr::BOTTOM,
                                     kit -> argument(0), kit -> argument(1), kit -> argument(2),
                                     kit -> argument(3), kit -> argument(4), kit -> argument(5),
                                     kit -> argument(6));
            break;
        case 8:
            kit -> make_runtime_call(_jni_flag, _jni_signature,
                                     stubAddr, stubName, TypePtr::BOTTOM,
                                     kit -> argument(0), kit -> argument(1), kit -> argument(2),
                                     kit -> argument(3), kit -> argument(4), kit -> argument(5),
                                     kit -> argument(6), kit -> argument(7));
            break;
    }
}
