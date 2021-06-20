#include "detect_vmware.h"

#include "cpuid.h"

#include <set>
#include <string_view>
#include <cstring>
#include <cstdint>

using namespace std::literals;
std::set<std::string_view> hyperv_vm_ids {
    "bhyve bhyve "sv,
    "KVMKVMKVM"sv,
    "Microsoft Hv"sv,
    "VMwareVMware"sv,
    "XenVMMXenVMM"sv
};

inline bool cpuid_check()
{
    int cpuid[4] { 0, 0, 0, 0 };

    __cpuid(cpuid, 1);

    char hyperv_id[13];

    if ((cpuid[2] & 1) != 0) {
        __cpuid(cpuid, 0x40000000);
        std::memcpy(hyperv_id + 0, cpuid + 1, 4);
        std::memcpy(hyperv_id + 4, cpuid + 2, 4);
        std::memcpy(hyperv_id + 8, cpuid + 3, 4);
        hyperv_id[12] = '\0';

        if (hyperv_vm_ids.contains(hyperv_id)) {
            return true; // Success - running under VMware
        }
    }

    return false;
}
/*
inline bool dmi_check() {
    char string[10];
    GET_BIOS_SERIAL(string);
    if (!std::memcmp(string, "VMware-", 7) || !std::memcmp(string, "VMW", 3)) {
        return true;
    }

    return false;
}

#define VMWARE_HYPERVISOR_MAGIC 0x564D5868
#define VMWARE_HYPERVISOR_PORT  0x5658
#define VMWARE_PORT_CMD_GETVERSION 10
#define VMWARE_PORT(cmd, eax, ebx, ecx, edx)        \
    __asm__("inl (%%dx)" :                          \
    "=a"(eax), "=c"(ecx), "=d"(edx), "=b"(ebx) :    \
    "0"(VMWARE_HYPERVISOR_MAGIC),                   \
    "1"(VMWARE_PORT_CMD_##cmd),                     \
    "2"(VMWARE_HYPERVISOR_PORT), "3"(UINT_MAX) :    \
    "memory");

int hypervisor_port_check(void) {
    std::uint32_t eax, ebx, ecx, edx;
    VMWARE_PORT(GETVERSION, eax, ebx, ecx, edx);
    if (ebx == VMWARE_HYPERVISOR_MAGIC) {
        return true;
    }

    return false;
}
*/
bool ThorQ::Security::IsInsideVMWare()
{
    if (cpuid_check()) {
        return true;
    }
/*
    if (dmi_check() && hypervisor_port_check()) {
        return true;
    }
*/
    return false;
}
