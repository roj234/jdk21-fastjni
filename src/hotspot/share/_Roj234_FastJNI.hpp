//
// Created by Roj234 on 2024/9/24 0024.
//

#include <cstdint>
#include <iostream>

#define FastJNICALL __stdcall

struct FastJNIMethod {
    const char *owner, *name, *desc;

    const char *_desc;
    uint8_t _descLen;

    const TypeFunc *_signature;

    address _address;
} ;

static FastJNIMethod methods[256];

FastJNICALL int32_t myMethod(int32_t arg1) {
    return arg1 * 233;
}

address AsmLinker1() {
    _masm-> align(CodeEntryAlignment);
    //StubCodeMark mark(this, "StubRoutines", "generate_MyMethod");
    address start = _masm-> pc();

    const Register ra0 = c_rarg0;
    const Register ra1 = c_rarg1;
    const Register ra2 = c_rarg2;
    const Register ra3 = c_rarg3;

    _masm-> enter();
    myMethod(ra0);
    _masm-> leave(); // required for proper stackwalking of RuntimeStub frame
    _masm-> ret(0);
    return start;
}

static FastJNIMethod TEST = {
        "roj/FastJNI", "test",  "(I)I",
        "II", 1,
        nullptr,
        reinterpret_cast<address>(&myMethod)
};

bool LinkFastJni(GraphKit *kit, FastJNIMethod* id);
FastJNIMethod* FindFastJni(const char* owner, const char* name, const char* desc);

JVMState* LibraryIntrinsic::generate(JVMState* jvms) {
  LibraryCallKit kit(jvms, this);
  ciMethod* callee = kit.callee();
  // Try to inline the intrinsic.
  if (true/*callee->check_intrinsic_candidate_my()*/) {
    const char *owner, *name, *desc;
    owner = callee->holder()->name()->as_utf8();
    name = callee->name()->as_utf8();
    desc = callee->signature()->as_symbol()->as_utf8();

    FastJNIMethod* fastJni = FindFastJni(owner, name, desc);
    if (fastJni && LinkFastJni(kit, fastJni)) {
        kit.push_result();
        return kit.transfer_exceptions_into_jvms();
    }
  }

  return this->_generate(predicate);
}

FastJNIMethod* FindFastJni(const char* owner, const char* name, const char* desc) {
//#ifndef PRODUCT
    tty->print_cr("FastJNI Try %s.%s%s", owner, name, desc);
//#endif
    //std::cout << "FastJNI " << owner << "." << name << " " << desc << "\n";
    return strcmp(name, "callNative") != 0) ? nullptr : &TEST;
}

bool LinkFastJni(LibraryCallKit *kit, FastJNIMethod* id) {
    const TypeFunc *_jni_signature;
    if (id->_signature == nullptr) {
#define AAAA \
	switch (id->_desc[i]) { \
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
        const Type** fields = TypeTuple::fields(id->_descLen);
        uint8_t i=0;
        const Type *jType;

        int argp = TypeFunc::Parms;
        for (; i < id->_descLen; i++) {
            AAAA
            fields[argp++] = jType;
        }
        const TypeTuple* domain = TypeTuple::make(TypeFunc::Parms+id->_descLen, fields);

        fields = TypeTuple::fields(1);
        AAAA
        fields[TypeFunc::Parms+0] = jType; // void
        const TypeTuple* range = TypeTuple::make(TypeFunc::Parms, fields);

        _jni_signature = id->_signature = TypeFunc::make(domain, range);

#undef AAAA
    } else {
        _jni_signature = id->_signature;
    }

    // FastJNI
    address stubAddr;
    if (id->_address == nullptr) {
        //stubAddr = //...;
        id->_address = stubAddr;
        if (stubAddr == nullptr) return false;
    } else {
        stubAddr = id->_address;
    }

    const char *stubName = id->name;

    int _jni_flag = LibraryCallKit::RC_LEAF | LibraryCallKit::RC_NO_FP;
    switch (id->_descLen) {
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
    return true;
}
