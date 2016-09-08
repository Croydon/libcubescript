#include "cubescript/cubescript.hh"
#include "cs_vm.hh"

namespace cscript {

CsString intstr(CsInt v) {
    char buf[256];
    snprintf(buf, sizeof(buf), IntFormat, v);
    return static_cast<char const *>(buf);
}

CsString floatstr(CsFloat v) {
    char buf[256];
    snprintf(buf, sizeof(buf), v == CsInt(v) ? RoundFloatFormat : FloatFormat, v);
    return static_cast<char const *>(buf);
}

bool cs_check_num(ostd::ConstCharRange s) {
    if (isdigit(s[0])) {
        return true;
    }
    switch (s[0]) {
        case '+':
        case '-':
            return isdigit(s[1]) || ((s[1] == '.') && isdigit(s[2]));
        case '.':
            return isdigit(s[1]) != 0;
        default:
            return false;
    }
}

CsIdent::CsIdent(CsIdentType tp, ostd::ConstCharRange nm, int fl):
    p_name(nm), p_type(int(tp)), p_flags(fl)
{}

CsVar::CsVar(CsIdentType tp, ostd::ConstCharRange name, CsVarCb f, int fl):
    CsIdent(tp, name, fl), cb_var(ostd::move(f))
{}

CsIvar::CsIvar(
    ostd::ConstCharRange name, CsInt m, CsInt x, CsInt v, CsVarCb f, int fl
):
    CsVar(CsIdentType::Ivar, name, ostd::move(f), fl | ((m > x) ? CsIdfReadOnly : 0)),
    p_storage(v), p_minval(m), p_maxval(x), p_overrideval(0)
{}

CsFvar::CsFvar(
    ostd::ConstCharRange name, CsFloat m, CsFloat x, CsFloat v, CsVarCb f, int fl
):
    CsVar(CsIdentType::Fvar, name, ostd::move(f), fl | ((m > x) ? CsIdfReadOnly : 0)),
    p_storage(v), p_minval(m), p_maxval(x), p_overrideval(0)
{}

CsSvar::CsSvar(ostd::ConstCharRange name, CsString v, CsVarCb f, int fl):
    CsVar(CsIdentType::Svar, name, ostd::move(f), fl),
    p_storage(ostd::move(v)), p_overrideval()
{}

CsAlias::CsAlias(ostd::ConstCharRange name, char *a, int fl):
    CsIdent(CsIdentType::Alias, name, fl),
    p_acode(nullptr), p_astack(nullptr)
{
    p_val.set_mstr(a);
}
CsAlias::CsAlias(ostd::ConstCharRange name, CsInt a, int fl):
    CsIdent(CsIdentType::Alias, name, fl),
    p_acode(nullptr), p_astack(nullptr)
{
    p_val.set_int(a);
}
CsAlias::CsAlias(ostd::ConstCharRange name, CsFloat a, int fl):
    CsIdent(CsIdentType::Alias, name, fl),
    p_acode(nullptr), p_astack(nullptr)
{
    p_val.set_float(a);
}
CsAlias::CsAlias(ostd::ConstCharRange name, int fl):
    CsIdent(CsIdentType::Alias, name, fl),
    p_acode(nullptr), p_astack(nullptr)
{
    p_val.set_null();
}
CsAlias::CsAlias(ostd::ConstCharRange name, CsValue v, int fl):
    CsIdent(CsIdentType::Alias, name, fl),
    p_acode(nullptr), p_astack(nullptr), p_val(ostd::move(v))
{}

CsCommand::CsCommand(
    ostd::ConstCharRange name, ostd::ConstCharRange args,
    int nargs, CsCommandCb f
):
    CsIdent(CsIdentType::Command, name, 0),
    p_cargs(args), p_cb_cftv(ostd::move(f)), p_numargs(nargs)
{}

bool CsIdent::is_alias() const {
    return get_type() == CsIdentType::Alias;
}

CsAlias *CsIdent::get_alias() {
    if (!is_alias()) {
        return nullptr;
    }
    return static_cast<CsAlias *>(this);
}

CsAlias const *CsIdent::get_alias() const {
    if (!is_alias()) {
        return nullptr;
    }
    return static_cast<CsAlias const *>(this);
}

bool CsIdent::is_command() const {
    return get_type() == CsIdentType::Command;
}

CsCommand *CsIdent::get_command() {
    if (!is_command()) {
        return nullptr;
    }
    return static_cast<CsCommand *>(this);
}

CsCommand const *CsIdent::get_command() const {
    if (!is_command()) {
        return nullptr;
    }
    return static_cast<CsCommand const *>(this);
}

bool CsIdent::is_special() const {
    return get_type() == CsIdentType::Special;
}

bool CsIdent::is_var() const {
    CsIdentType tp = get_type();
    return (tp >= CsIdentType::Ivar) && (tp <= CsIdentType::Svar);
}

CsVar *CsIdent::get_var() {
    if (!is_var()) {
        return nullptr;
    }
    return static_cast<CsVar *>(this);
}

CsVar const *CsIdent::get_var() const {
    if (!is_var()) {
        return nullptr;
    }
    return static_cast<CsVar const *>(this);
}

bool CsIdent::is_ivar() const {
    return get_type() == CsIdentType::Ivar;
}

CsIvar *CsIdent::get_ivar() {
    if (!is_ivar()) {
        return nullptr;
    }
    return static_cast<CsIvar *>(this);
}

CsIvar const *CsIdent::get_ivar() const {
    if (!is_ivar()) {
        return nullptr;
    }
    return static_cast<CsIvar const *>(this);
}

bool CsIdent::is_fvar() const {
    return get_type() == CsIdentType::Fvar;
}

CsFvar *CsIdent::get_fvar() {
    if (!is_fvar()) {
        return nullptr;
    }
    return static_cast<CsFvar *>(this);
}

CsFvar const *CsIdent::get_fvar() const {
    if (!is_fvar()) {
        return nullptr;
    }
    return static_cast<CsFvar const *>(this);
}

bool CsIdent::is_svar() const {
    return get_type() == CsIdentType::Svar;
}

CsSvar *CsIdent::get_svar() {
    if (!is_svar()) {
        return nullptr;
    }
    return static_cast<CsSvar *>(this);
}

CsSvar const *CsIdent::get_svar() const {
    if (!is_svar()) {
        return nullptr;
    }
    return static_cast<CsSvar const *>(this);
}

CsInt CsIvar::get_val_min() const {
    return p_minval;
}
CsInt CsIvar::get_val_max() const {
    return p_maxval;
}

CsInt CsIvar::get_value() const {
    return p_storage;
}
void CsIvar::set_value(CsInt val) {
    p_storage = val;
}

CsFloat CsFvar::get_val_min() const {
    return p_minval;
}
CsFloat CsFvar::get_val_max() const {
    return p_maxval;
}

CsFloat CsFvar::get_value() const {
    return p_storage;
}
void CsFvar::set_value(CsFloat val) {
    p_storage = val;
}

ostd::ConstCharRange CsSvar::get_value() const {
    return p_storage.iter();
}
void CsSvar::set_value(CsString val) {
    p_storage = ostd::move(val);
}

ostd::ConstCharRange CsCommand::get_args() const {
    return p_cargs;
}

int CsCommand::get_num_args() const {
    return p_numargs;
}

void cs_init_lib_base(CsState &cs);

CsState::CsState():
    p_callhook(), p_panicfunc(), p_out(&ostd::out), p_err(&ostd::err)
{
    noalias.id = nullptr;
    noalias.next = nullptr;
    noalias.usedargs = (1 << MaxArguments) - 1;
    noalias.argstack = nullptr;

    /* default panic func */
    p_panicfunc = [this](CsString v) {
        get_err().writefln(
            "PANIC: unprotected error in call to CubeScript (%s)", v
        );
    };

    for (int i = 0; i < MaxArguments; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "arg%d", i + 1);
        new_ident(static_cast<char const *>(buf), CsIdfArg);
    }
    CsIdent *id = new_ident("//dummy");
    assert(id->get_index() == DummyIdx);

    id = new_ivar("numargs", MaxArguments, 0, 0);
    assert(id->get_index() == NumargsIdx);

    id = new_ivar("dbgalias", 0, 1000, 4);
    assert(id->get_index() == DbgaliasIdx);

    new_command("do", "e", [this](CsValueRange args, CsValue &res) {
        run(args[0].get_code(), res);
    })->p_type = CsIdDo;

    new_command("doargs", "e", [this](CsValueRange args, CsValue &res) {
        if (p_callstack != &noalias) {
            cs_do_args(*this, [&]() { run(args[0].get_code(), res); });
        } else {
            run(args[0].get_code(), res);
        }
    })->p_type = CsIdDoArgs;

    new_command("if", "tee", [this](CsValueRange args, CsValue &res) {
        run((args[0].get_bool() ? args[1] : args[2]).get_code(), res);
    })->p_type = CsIdIf;

    new_command("result", "T", [](CsValueRange args, CsValue &res) {
        res = ostd::move(args[0]);
    })->p_type = CsIdResult;

    new_command("!", "t", [](CsValueRange args, CsValue &res) {
        res.set_int(!args[0].get_bool());
    })->p_type = CsIdNot;

    new_command("&&", "E1V", [this](CsValueRange args, CsValue &res) {
        if (args.empty()) {
            res.set_int(1);
        } else {
            for (ostd::Size i = 0; i < args.size(); ++i) {
                CsBytecode *code = args[i].get_code();
                if (code) {
                    run(code, res);
                } else {
                    res = ostd::move(args[i]);
                }
                if (!res.get_bool()) {
                    break;
                }
            }
        }
    })->p_type = CsIdAnd;

    new_command("||", "E1V", [this](CsValueRange args, CsValue &res) {
        if (args.empty()) {
            res.set_int(0);
        } else {
            for (ostd::Size i = 0; i < args.size(); ++i) {
                CsBytecode *code = args[i].get_code();
                if (code) {
                    run(code, res);
                } else {
                    res = ostd::move(args[i]);
                }
                if (res.get_bool()) {
                    break;
                }
            }
        }
    })->p_type = CsIdOr;

    new_command("local", nullptr, nullptr)->p_type = CsIdLocal;

    cs_init_lib_base(*this);
}

CsState::~CsState() {
    for (auto &p: idents.iter()) {
        CsIdent *i = p.second;
        CsAlias *a = i->get_alias();
        if (a) {
            a->get_value().force_null();
            CsAliasInternal::clean_code(a);
        }
        delete i;
    }
}

CsStream const &CsState::get_out() const {
    return *p_out;
}

CsStream &CsState::get_out() {
    return *p_out;
}

void CsState::set_out(CsStream &s) {
    p_out = &s;
}

CsStream const &CsState::get_err() const {
    return *p_err;
}

CsStream &CsState::get_err() {
    return *p_err;
}

void CsState::set_err(CsStream &s) {
    p_err = &s;
}

CsHookCb CsState::set_call_hook(CsHookCb func) {
    auto hk = ostd::move(p_callhook);
    p_callhook = ostd::move(func);
    return hk;
}

CsHookCb const &CsState::get_call_hook() const {
    return p_callhook;
}

CsHookCb &CsState::get_call_hook() {
    return p_callhook;
}

void *CsState::alloc(void *ptr, ostd::Size, ostd::Size ns) {
    delete[] static_cast<ostd::byte *>(ptr);
    if (!ns) {
        return nullptr;
    }
    return new ostd::byte[ns];
}

CsPanicCb CsState::set_panic_func(CsPanicCb func) {
    auto hk = ostd::move(p_panicfunc);
    p_panicfunc = ostd::move(func);
    return hk;
}

CsPanicCb const &CsState::get_panic_func() const {
    return p_panicfunc;
}

CsPanicCb &CsState::get_panic_func() {
    return p_panicfunc;
}

bool CsState::ipcall(void (*f)(void *data), ostd::String *error, void *data) {
    ++protect;
    try {
        f(data);
    } catch (CsErrorException const &v) {
        --protect;
        if (error) {
            *error = ostd::move(v.errmsg);
        }
        return false;
    } catch (...) {
        --protect;
        throw;
    }
    --protect;
    return true;
}

void CsState::error(CsString msg) {
    if (protect) {
        throw CsErrorException(ostd::move(msg));
    } else {
        if (p_panicfunc) {
            p_panicfunc(ostd::move(msg));
        }
        exit(EXIT_FAILURE);
    }
}

void CsState::clear_override(CsIdent &id) {
    if (!(id.get_flags() & CsIdfOverridden)) {
        return;
    }
    switch (id.get_type()) {
        case CsIdentType::Alias: {
            CsAlias &a = static_cast<CsAlias &>(id);
            CsAliasInternal::clean_code(&a);
            a.get_value().set_str("");
            break;
        }
        case CsIdentType::Ivar: {
            CsIvar &iv = static_cast<CsIvar &>(id);
            iv.set_value(iv.p_overrideval);
            iv.changed();
            break;
        }
        case CsIdentType::Fvar: {
            CsFvar &fv = static_cast<CsFvar &>(id);
            fv.set_value(fv.p_overrideval);
            fv.changed();
            break;
        }
        case CsIdentType::Svar: {
            CsSvar &sv = static_cast<CsSvar &>(id);
            sv.set_value(sv.p_overrideval);
            sv.changed();
            break;
        }
        default:
            break;
    }
    id.p_flags &= ~CsIdfOverridden;
}

void CsState::clear_overrides() {
    for (auto &p: idents.iter()) {
        clear_override(*(p.second));
    }
}

CsIdent *CsState::add_ident(CsIdent *id) {
    if (!id) {
        return nullptr;
    }
    idents[id->get_name()] = id;
    id->p_index = identmap.size();
    return identmap.push(id);
}

CsIdent *CsState::new_ident(ostd::ConstCharRange name, int flags) {
    CsIdent *id = get_ident(name);
    if (!id) {
        if (cs_check_num(name)) {
            cs_debug_code(
                *this, "number %s is not a valid identifier name", name
            );
            return identmap[DummyIdx];
        }
        id = add_ident(new CsAlias(name, flags));
    }
    return id;
}

CsIdent *CsState::force_ident(CsValue &v) {
    switch (v.get_type()) {
        case CsValueType::Ident:
            return v.get_ident();
        case CsValueType::Macro:
        case CsValueType::Cstring:
        case CsValueType::String: {
            CsIdent *id = new_ident(v.get_strr());
            v.set_ident(id);
            return id;
        }
        default:
            break;
    }
    v.set_ident(identmap[DummyIdx]);
    return identmap[DummyIdx];
}

CsIvar *CsState::new_ivar(
    ostd::ConstCharRange n, CsInt m, CsInt x, CsInt v, CsVarCb f, int flags
) {
    return add_ident(new CsIvar(n, m, x, v, ostd::move(f), flags))->get_ivar();
}

CsFvar *CsState::new_fvar(
    ostd::ConstCharRange n, CsFloat m, CsFloat x, CsFloat v, CsVarCb f, int flags
) {
    return add_ident(new CsFvar(n, m, x, v, ostd::move(f), flags))->get_fvar();
}

CsSvar *CsState::new_svar(
    ostd::ConstCharRange n, CsString v, CsVarCb f, int flags
) {
    return add_ident(
        new CsSvar(n, ostd::move(v), ostd::move(f), flags)
    )->get_svar();
}

bool CsState::reset_var(ostd::ConstCharRange name) {
    CsIdent *id = get_ident(name);
    if (!id) {
        return false;
    }
    if (id->get_flags() & CsIdfReadOnly) {
        cs_debug_code(*this, "variable %s is read only", id->get_name());
        return false;
    }
    clear_override(*id);
    return true;
}

void CsState::touch_var(ostd::ConstCharRange name) {
    CsIdent *id = get_ident(name);
    if (id && id->is_var()) {
        static_cast<CsVar *>(id)->changed();
    }
}

void CsState::set_alias(ostd::ConstCharRange name, CsValue v) {
    CsIdent *id = get_ident(name);
    if (id) {
        switch (id->get_type()) {
            case CsIdentType::Alias: {
                CsAlias *a = static_cast<CsAlias *>(id);
                if (a->get_index() < MaxArguments) {
                    CsAliasInternal::set_arg(a, *this, v);
                } else {
                    CsAliasInternal::set_alias(a, *this, v);
                }
                return;
            }
            case CsIdentType::Ivar:
                set_var_int_checked(static_cast<CsIvar *>(id), v.get_int());
                break;
            case CsIdentType::Fvar:
                set_var_float_checked(static_cast<CsFvar *>(id), v.get_float());
                break;
            case CsIdentType::Svar:
                set_var_str_checked(static_cast<CsSvar *>(id), v.get_str());
                break;
            default:
                cs_debug_code(
                    *this, "cannot redefine builtin %s with an alias",
                    id->get_name()
                );
                break;
        }
    } else if (cs_check_num(name)) {
        cs_debug_code(*this, "cannot alias number %s", name);
    } else {
        add_ident(new CsAlias(name, ostd::move(v), identflags));
    }
}

void CsState::print_var(CsIvar *iv) {
    CsInt i = iv->get_value();
    if (i < 0) {
        get_out().writefln("%s = %d", iv->get_name(), i);
        return;
    }
    if (iv->get_flags() & CsIdfHex) {
        if (iv->get_val_max() == 0xFFFFFF) {
            get_out().writefln(
                "%s = 0x%.6X (%d, %d, %d)", iv->get_name(),
                i, (i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF
            );
        } else {
            get_out().writefln("%s = 0x%X", iv->get_name(), i);
        }
    } else {
        get_out().writefln("%s = %d", iv->get_name(), i);
    }
}

void CsState::print_var(CsFvar *fv) {
    get_out().writefln("%s = %s", fv->get_name(), floatstr(fv->get_value()));
}

void CsState::print_var(CsSvar *sv) {
    ostd::ConstCharRange sval = sv->get_value();
    if (ostd::find(sval, '"').empty()) {
        get_out().writefln("%s = \"%s\"", sv->get_name(), sval);
    } else {
        get_out().writefln("%s = [%s]", sv->get_name(), sval);
    }
}

void CsState::print_var(CsVar *v) {
    switch (v->get_type()) {
        case CsIdentType::Ivar: {
            CsIvar *iv = static_cast<CsIvar *>(v);
            print_var(iv);
            break;
        }
        case CsIdentType::Fvar: {
            CsFvar *fv = static_cast<CsFvar *>(v);
            print_var(fv);
            break;
        }
        case CsIdentType::Svar: {
            CsSvar *sv = static_cast<CsSvar *>(v);
            print_var(sv);
            break;
        }
        default:
            break;
    }
}

void CsAlias::get_cstr(CsValue &v) const {
    switch (p_val.get_type()) {
        case CsValueType::Macro:
            v.set_macro(p_val.get_strr());
            break;
        case CsValueType::String:
        case CsValueType::Cstring:
            v.set_cstr(p_val.get_strr());
            break;
        case CsValueType::Int:
            v.set_str(ostd::move(intstr(p_val.get_int())));
            break;
        case CsValueType::Float:
            v.set_str(ostd::move(floatstr(p_val.get_float())));
            break;
        default:
            v.set_cstr("");
            break;
    }
}

void CsAlias::get_cval(CsValue &v) const {
    switch (p_val.get_type()) {
        case CsValueType::Macro:
            v.set_macro(p_val.get_strr());
            break;
        case CsValueType::String:
        case CsValueType::Cstring:
            v.set_cstr(p_val.get_strr());
            break;
        case CsValueType::Int:
            v.set_int(p_val.get_int());
            break;
        case CsValueType::Float:
            v.set_float(p_val.get_float());
            break;
        default:
            v.set_null();
            break;
    }
}

CsIdentType CsIdent::get_type() const {
    if (p_type > CsIdAlias) {
        return CsIdentType::Special;
    }
    return CsIdentType(p_type);
}

ostd::ConstCharRange CsIdent::get_name() const {
    return p_name;
}

int CsIdent::get_flags() const {
    return p_flags;
}

int CsIdent::get_index() const {
    return p_index;
}

template<typename SF>
static inline bool cs_override_var(CsState &cs, CsVar *v, int &vflags, SF sf) {
    if ((cs.identflags & CsIdfOverridden) || (vflags & CsIdfOverride)) {
        if (vflags & CsIdfPersist) {
            cs_debug_code(
                cs, "cannot override persistent variable '%s'", v->get_name()
            );
            return false;
        }
        if (!(vflags & CsIdfOverridden)) {
            sf();
            vflags |= CsIdfOverridden;
        }
    } else {
        if (vflags & CsIdfOverridden) {
            vflags &= ~CsIdfOverridden;
        }
    }
    return true;
}

void CsState::set_var_int(
    ostd::ConstCharRange name, CsInt v, bool dofunc, bool doclamp
) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_ivar()) {
        return;
    }
    CsIvar *iv = static_cast<CsIvar *>(id);
    bool success = cs_override_var(
        *this, iv, iv->p_flags,
        [&iv]() { iv->p_overrideval = iv->get_value(); }
    );
    if (!success) {
        return;
    }
    if (doclamp) {
        iv->set_value(ostd::clamp(v, iv->get_val_min(), iv->get_val_max()));
    } else {
        iv->set_value(v);
    }
    if (dofunc) {
        iv->changed();
    }
}

void CsState::set_var_float(
    ostd::ConstCharRange name, CsFloat v, bool dofunc, bool doclamp
) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_fvar()) {
        return;
    }
    CsFvar *fv = static_cast<CsFvar *>(id);
    bool success = cs_override_var(
        *this, fv, fv->p_flags,
        [&fv]() { fv->p_overrideval = fv->get_value(); }
    );
    if (!success) {
        return;
    }
    if (doclamp) {
        fv->set_value(ostd::clamp(v, fv->get_val_min(), fv->get_val_max()));
    } else {
        fv->set_value(v);
    }
    if (dofunc) {
        fv->changed();
    }
}

void CsState::set_var_str(
    ostd::ConstCharRange name, ostd::ConstCharRange v, bool dofunc
) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_svar()) {
        return;
    }
    CsSvar *sv = static_cast<CsSvar *>(id);
    bool success = cs_override_var(
        *this, sv, sv->p_flags,
        [&sv]() { sv->p_overrideval = sv->get_value(); }
    );
    if (!success) {
        return;
    }
    sv->set_value(v);
    if (dofunc) {
        sv->changed();
    }
}

ostd::Maybe<CsInt> CsState::get_var_int(ostd::ConstCharRange name) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_ivar()) {
        return ostd::nothing;
    }
    return static_cast<CsIvar *>(id)->get_value();
}

ostd::Maybe<CsFloat> CsState::get_var_float(ostd::ConstCharRange name) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_fvar()) {
        return ostd::nothing;
    }
    return static_cast<CsFvar *>(id)->get_value();
}

ostd::Maybe<CsString> CsState::get_var_str(ostd::ConstCharRange name) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_svar()) {
        return ostd::nothing;
    }
    return CsString(static_cast<CsSvar *>(id)->get_value());
}

ostd::Maybe<CsInt> CsState::get_var_min_int(ostd::ConstCharRange name) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_ivar()) {
        return ostd::nothing;
    }
    return static_cast<CsIvar *>(id)->get_val_min();
}

ostd::Maybe<CsInt> CsState::get_var_max_int(ostd::ConstCharRange name) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_ivar()) {
        return ostd::nothing;
    }
    return static_cast<CsIvar *>(id)->get_val_max();
}

ostd::Maybe<CsFloat> CsState::get_var_min_float(ostd::ConstCharRange name) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_fvar()) {
        return ostd::nothing;
    }
    return static_cast<CsFvar *>(id)->get_val_min();
}

ostd::Maybe<CsFloat> CsState::get_var_max_float(ostd::ConstCharRange name) {
    CsIdent *id = get_ident(name);
    if (!id || id->is_fvar()) {
        return ostd::nothing;
    }
    return static_cast<CsFvar *>(id)->get_val_max();
}

ostd::Maybe<CsString>
CsState::get_alias_val(ostd::ConstCharRange name) {
    CsAlias *a = get_alias(name);
    if (!a) {
        return ostd::nothing;
    }
    if (
        (a->get_index() < MaxArguments) &&
        !(p_callstack->usedargs & (1 << a->get_index()))
    ) {
        return ostd::nothing;
    }
    return ostd::move(a->get_value().get_str());
}

CsInt cs_clamp_var(CsState &cs, CsIvar *iv, CsInt v) {
    if (v < iv->get_val_min()) {
        v = iv->get_val_min();
    } else if (v > iv->get_val_max()) {
        v = iv->get_val_max();
    } else {
        return v;
    }
    cs_debug_code(
        cs,
        (iv->get_flags() & CsIdfHex)
            ? (
                (iv->get_val_min() <= 255)
                    ? "valid range for '%s' is %d..0x%X"
                    : "valid range for '%s' is 0x%X..0x%X"
            )
            : "valid range for '%s' is %d..%d",
        iv->get_name(), iv->get_val_min(), iv->get_val_max()
    );
    return v;
}

void CsState::set_var_int_checked(CsIvar *iv, CsInt v) {
    if (iv->get_flags() & CsIdfReadOnly) {
        cs_debug_code(*this, "variable '%s' is read only", iv->get_name());
        return;
    }
    bool success = cs_override_var(
        *this, iv, iv->p_flags,
        [&iv]() { iv->p_overrideval = iv->get_value(); }
    );
    if (!success) {
        return;
    }
    if ((v < iv->get_val_min()) || (v > iv->get_val_max())) {
        v = cs_clamp_var(*this, iv, v);
    }
    iv->set_value(v);
    iv->changed();
}

void CsState::set_var_int_checked(CsIvar *iv, CsValueRange args) {
    CsInt v = args[0].force_int();
    if ((iv->get_flags() & CsIdfHex) && (args.size() > 1)) {
        v = (v << 16) | (args[1].force_int() << 8);
        if (args.size() > 2) {
            v |= args[2].force_int();
        }
    }
    set_var_int_checked(iv, v);
}

CsFloat cs_clamp_fvar(CsState &cs, CsFvar *fv, CsFloat v) {
    if (v < fv->get_val_min()) {
        v = fv->get_val_min();
    } else if (v > fv->get_val_max()) {
        v = fv->get_val_max();
    } else {
        return v;
    }
    cs_debug_code(
        cs, "valid range for '%s' is %s..%s", floatstr(fv->get_val_min()),
        floatstr(fv->get_val_max())
    );
    return v;
}

void CsState::set_var_float_checked(CsFvar *fv, CsFloat v) {
    if (fv->get_flags() & CsIdfReadOnly) {
        cs_debug_code(*this, "variable '%s' is read only", fv->get_name());
        return;
    }
    bool success = cs_override_var(
        *this, fv, fv->p_flags,
        [&fv]() { fv->p_overrideval = fv->get_value(); }
    );
    if (!success) {
        return;
    }
    if ((v < fv->get_val_min()) || (v > fv->get_val_max())) {
        v = cs_clamp_fvar(*this, fv, v);
    }
    fv->set_value(v);
    fv->changed();
}

void CsState::set_var_str_checked(CsSvar *sv, ostd::ConstCharRange v) {
    if (sv->get_flags() & CsIdfReadOnly) {
        cs_debug_code(*this, "variable '%s' is read only", sv->get_name());
        return;
    }
    bool success = cs_override_var(
        *this, sv, sv->p_flags,
        [&sv]() { sv->p_overrideval = sv->get_value(); }
    );
    if (!success) {
        return;
    }
    sv->set_value(v);
    sv->changed();
}

CsCommand *CsState::new_command(
    ostd::ConstCharRange name, ostd::ConstCharRange args, CsCommandCb func
) {
    int nargs = 0;
    for (ostd::ConstCharRange fmt(args); !fmt.empty(); ++fmt) {
        switch (*fmt) {
            case 'i':
            case 'b':
            case 'f':
            case 'F':
            case 't':
            case 'T':
            case 'E':
            case 'N':
            case 'S':
            case 's':
            case 'e':
            case 'r':
            case '$':
                if (nargs < MaxArguments) {
                    ++nargs;
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
                if (nargs < (*fmt - '0')) {
                    return nullptr;
                }
                if ((fmt.size() != 2) || ((fmt[1] != 'C') && (fmt[1] != 'V'))) {
                    return nullptr;
                }
                if (nargs < MaxArguments) {
                    fmt.push_front_n(*fmt - '0' + 1);
                }
                break;
            case 'C':
            case 'V':
                if (fmt.size() != 1) {
                    return nullptr;
                }
                break;
            default:
                return nullptr;
        }
    }
    return static_cast<CsCommand *>(
        add_ident(new CsCommand(name, args, nargs, ostd::move(func)))
    );
}

static inline void cs_do_loop(
    CsState &cs, CsIdent &id, CsInt offset, CsInt n, CsInt step,
    CsBytecode *cond, CsBytecode *body
) {
    CsStackedValue idv{&id};
    if (n <= 0 || !idv.has_alias()) {
        return;
    }
    for (CsInt i = 0; i < n; ++i) {
        idv.set_int(offset + i * step);
        idv.push();
        if (cond && !cs.run_bool(cond)) {
            break;
        }
        cs.run_int(body);
    }
}

static inline void cs_loop_conc(
    CsState &cs, CsValue &res, CsIdent &id, CsInt offset, CsInt n,
    CsInt step, CsBytecode *body, bool space
) {
    CsStackedValue idv{&id};
    if (n <= 0 || !idv.has_alias()) {
        return;
    }
    CsVector<char> s;
    for (CsInt i = 0; i < n; ++i) {
        idv.set_int(offset + i * step);
        idv.push();
        CsValue v;
        cs.run(body, v);
        CsString vstr = ostd::move(v.get_str());
        if (space && i) {
            s.push(' ');
        }
        s.push_n(vstr.data(), vstr.size());
    }
    s.push('\0');
    ostd::Size len = s.size() - 1;
    res.set_mstr(ostd::CharRange(s.disown(), len));
}

void cs_init_lib_base(CsState &cs) {
    cs.new_command("?", "tTT", [](CsValueRange args, CsValue &res) {
        if (args[0].get_bool()) {
            res = ostd::move(args[1]);
        } else {
            res = ostd::move(args[2]);
        }
    });

    cs.new_command("cond", "ee2V", [&cs](CsValueRange args, CsValue &res) {
        for (ostd::Size i = 0; i < args.size(); i += 2) {
            if ((i + 1) < args.size()) {
                if (cs.run_bool(args[i].get_code())) {
                    cs.run(args[i + 1].get_code(), res);
                    break;
                }
            } else {
                cs.run(args[i].get_code(), res);
                break;
            }
        }
    });

    cs.new_command("case", "ite2V", [&cs](CsValueRange args, CsValue &res) {
        CsInt val = args[0].get_int();
        for (ostd::Size i = 1; (i + 1) < args.size(); i += 2) {
            if (
                (args[i].get_type() == CsValueType::Null) ||
                (args[i].get_int() == val)
            ) {
                cs.run(args[i + 1].get_code(), res);
                return;
            }
        }
    });

    cs.new_command("casef", "fte2V", [&cs](CsValueRange args, CsValue &res) {
        CsFloat val = args[0].get_float();
        for (ostd::Size i = 1; (i + 1) < args.size(); i += 2) {
            if (
                (args[i].get_type() == CsValueType::Null) ||
                (args[i].get_float() == val)
            ) {
                cs.run(args[i + 1].get_code(), res);
                return;
            }
        }
    });

    cs.new_command("cases", "ste2V", [&cs](CsValueRange args, CsValue &res) {
        CsString val = args[0].get_str();
        for (ostd::Size i = 1; (i + 1) < args.size(); i += 2) {
            if (
                (args[i].get_type() == CsValueType::Null) ||
                (args[i].get_str() == val)
            ) {
                cs.run(args[i + 1].get_code(), res);
                return;
            }
        }
    });

    cs.new_command("pushif", "rTe", [&cs](CsValueRange args, CsValue &res) {
        CsStackedValue idv{args[0].get_ident()};
        if (!idv.has_alias() || (idv.get_alias()->get_index() < MaxArguments)) {
            return;
        }
        if (args[1].get_bool()) {
            idv = ostd::move(args[1]);
            idv.push();
            cs.run(args[2].get_code(), res);
        }
    });

    cs.new_command("loop", "rie", [&cs](CsValueRange args, CsValue &) {
        cs_do_loop(
            cs, *args[0].get_ident(), 0, args[1].get_int(), 1, nullptr,
            args[2].get_code()
        );
    });

    cs.new_command("loop+", "riie", [&cs](CsValueRange args, CsValue &) {
        cs_do_loop(
            cs, *args[0].get_ident(), args[1].get_int(), args[2].get_int(), 1,
            nullptr, args[3].get_code()
        );
    });

    cs.new_command("loop*", "riie", [&cs](CsValueRange args, CsValue &) {
        cs_do_loop(
            cs, *args[0].get_ident(), 0, args[1].get_int(), args[2].get_int(),
            nullptr, args[3].get_code()
        );
    });

    cs.new_command("loop+*", "riiie", [&cs](CsValueRange args, CsValue &) {
        cs_do_loop(
            cs, *args[0].get_ident(), args[1].get_int(), args[3].get_int(),
            args[2].get_int(), nullptr, args[4].get_code()
        );
    });

    cs.new_command("loopwhile", "riee", [&cs](CsValueRange args, CsValue &) {
        cs_do_loop(
            cs, *args[0].get_ident(), 0, args[1].get_int(), 1,
            args[2].get_code(), args[3].get_code()
        );
    });

    cs.new_command("loopwhile+", "riiee", [&cs](CsValueRange args, CsValue &) {
        cs_do_loop(
            cs, *args[0].get_ident(), args[1].get_int(), args[2].get_int(), 1,
            args[3].get_code(), args[4].get_code()
        );
    });

    cs.new_command("loopwhile*", "riiee", [&cs](CsValueRange args, CsValue &) {
        cs_do_loop(
            cs, *args[0].get_ident(), 0, args[2].get_int(), args[1].get_int(),
            args[3].get_code(), args[4].get_code()
        );
    });

    cs.new_command("loopwhile+*", "riiiee", [&cs](CsValueRange args, CsValue &) {
        cs_do_loop(
            cs, *args[0].get_ident(), args[1].get_int(), args[3].get_int(),
            args[2].get_int(), args[4].get_code(), args[5].get_code()
        );
    });

    cs.new_command("while", "ee", [&cs](CsValueRange args, CsValue &) {
        CsBytecode *cond = args[0].get_code(), *body = args[1].get_code();
        while (cs.run_bool(cond)) {
            cs.run_int(body);
        }
    });

    cs.new_command("loopconcat", "rie", [&cs](CsValueRange args, CsValue &res) {
        cs_loop_conc(
            cs, res, *args[0].get_ident(), 0, args[1].get_int(), 1,
            args[2].get_code(), true
        );
    });

    cs.new_command("loopconcat+", "riie", [&cs](CsValueRange args, CsValue &res) {
        cs_loop_conc(
            cs, res, *args[0].get_ident(), args[1].get_int(), args[2].get_int(), 1,
            args[3].get_code(), true
        );
    });

    cs.new_command("loopconcat*", "riie", [&cs](CsValueRange args, CsValue &res) {
        cs_loop_conc(
            cs, res, *args[0].get_ident(), 0, args[2].get_int(), args[1].get_int(),
            args[3].get_code(), true
        );
    });

    cs.new_command("loopconcat+*", "riiie", [&cs](CsValueRange args, CsValue &res) {
        cs_loop_conc(
            cs, res, *args[0].get_ident(), args[1].get_int(), args[3].get_int(),
            args[2].get_int(), args[4].get_code(), true
        );
    });

    cs.new_command("loopconcatword", "rie", [&cs](CsValueRange args, CsValue &res) {
        cs_loop_conc(
            cs, res, *args[0].get_ident(), 0, args[1].get_int(), 1,
            args[2].get_code(), false
        );
    });

    cs.new_command("loopconcatword+", "riie", [&cs](CsValueRange args, CsValue &res) {
        cs_loop_conc(
            cs, res, *args[0].get_ident(), args[1].get_int(), args[2].get_int(), 1,
            args[3].get_code(), false
        );
    });

    cs.new_command("loopconcatword*", "riie", [&cs](CsValueRange args, CsValue &res) {
        cs_loop_conc(
            cs, res, *args[0].get_ident(), 0, args[2].get_int(), args[1].get_int(),
            args[3].get_code(), false
        );
    });

    cs.new_command("loopconcatword+*", "riiie", [&cs](CsValueRange args, CsValue &res) {
        cs_loop_conc(
            cs, res, *args[0].get_ident(), args[1].get_int(), args[3].get_int(),
            args[2].get_int(), args[4].get_code(), false
        );
    });

    cs.new_command("nodebug", "e", [&cs](CsValueRange args, CsValue &res) {
        ++cs.nodebug;
        cs_do_and_cleanup([&]() {
            cs.run(args[0].get_code(), res);
        }, [&cs]() {
            --cs.nodebug;
        });
    });

    cs.new_command("push", "rTe", [&cs](CsValueRange args, CsValue &res) {
        CsStackedValue idv{args[0].get_ident()};
        if (!idv.has_alias() || (idv.get_alias()->get_index() < MaxArguments)) {
            return;
        }
        idv = ostd::move(args[1]);
        idv.push();
        cs.run(args[2].get_code(), res);
    });

    cs.new_command("resetvar", "s", [&cs](CsValueRange args, CsValue &res) {
        res.set_int(cs.reset_var(args[0].get_strr()));
    });

    cs.new_command("alias", "sT", [&cs](CsValueRange args, CsValue &) {
        cs.set_alias(args[0].get_strr(), ostd::move(args[1]));
    });

    cs.new_command("getvarmin", "s", [&cs](CsValueRange args, CsValue &res) {
        res.set_int(cs.get_var_min_int(args[0].get_strr()).value_or(0));
    });
    cs.new_command("getvarmax", "s", [&cs](CsValueRange args, CsValue &res) {
        res.set_int(cs.get_var_max_int(args[0].get_strr()).value_or(0));
    });
    cs.new_command("getfvarmin", "s", [&cs](CsValueRange args, CsValue &res) {
        res.set_float(cs.get_var_min_float(args[0].get_strr()).value_or(0.0f));
    });
    cs.new_command("getfvarmax", "s", [&cs](CsValueRange args, CsValue &res) {
        res.set_float(cs.get_var_max_float(args[0].get_strr()).value_or(0.0f));
    });

    cs.new_command("identexists", "s", [&cs](CsValueRange args, CsValue &res) {
        res.set_int(cs.have_ident(args[0].get_strr()));
    });

    cs.new_command("getalias", "s", [&cs](CsValueRange args, CsValue &res) {
        res.set_str(ostd::move(cs.get_alias_val(args[0].get_strr()).value_or("")));
    });
}

void cs_init_lib_math(CsState &cs);
void cs_init_lib_string(CsState &cs);
void cs_init_lib_list(CsState &cs);

OSTD_EXPORT void CsState::init_libs(int libs) {
    if (libs & CsLibMath) {
        cs_init_lib_math(*this);
    }
    if (libs & CsLibString) {
        cs_init_lib_string(*this);
    }
    if (libs & CsLibList) {
        cs_init_lib_list(*this);
    }
}

} /* namespace cscript */
