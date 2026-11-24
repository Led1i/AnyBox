#include "Fixture.hpp"
#include "Fixtures.hpp"

void FlowTests::CheckSwitches() {
    for (unsigned selector = 0; selector < 16; ++selector) {
        Fixture fixture;
        auto* entry = fixture.Block();
        auto* merge = fixture.Block();
        auto* shared = fixture.Block();
        auto* selection = fixture.Emit<LLVMBC::SwitchInst>(entry, fixture.Constant(selector), merge, 13);
        for (unsigned key = 0; key < 13; ++key) selection->addCase(fixture.Constant(key), key % 2 == 0 ? merge : shared);
        fixture.Emit<LLVMBC::BranchInst>(shared, merge);
        auto* phi = fixture.Emit<LLVMBC::PHINode>(merge, LLVMBC::Type::getInt16Ty(fixture.Context()), 9);
        for (unsigned index = 0; index < 8; ++index) phi->add_incoming(fixture.Constant(100), entry);
        phi->add_incoming(fixture.Constant(200), shared);
        fixture.Emit<LLVMBC::ReturnInst>(merge, nullptr);
        auto output = fixture.Lower();
        auto expected = selector < 13 && selector % 2 != 0 ? 200u : 100u;
        Fixtures::Require(Evaluate(output) == std::vector<std::uint32_t>{expected});
    }
}
