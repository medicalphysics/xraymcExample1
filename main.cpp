
// Example of a simple usecase of xraymc
// This example run a monte carlo simulation of a
// pecil beam directed onto a cylinder to obtain depthdose data

// Include xraymc header files
#include "xraymc/xraymc.hpp"

#include <iostream>

int main()
{
    // We start by creating some names, xraymc is a template library and typically there are a couple of
    // compile time parameters to be set at an item that is included in the world
    // All items included in xraymc

    using Beam = xraymc::PencilBeam<>;
    using DepthDose = xraymc::DepthDose<>;
    using World = xraymc::World<DepthDose>;
    using Material = xraymc::Material<>;

    World world;
    world.reserveNumberOfItems(1);

    auto &item = world.addItem<DepthDose>();

    item.setRadius(2);
    item.setDirection({0, 0, -1});
    item.setLenght(10);
    item.setResolution(100);

    // Material factory
    auto aluminum_material = Material::byZ(13).value();
    auto aluminum_density = xraymc::AtomHandler::Atom(13).standardDensity;

    auto pmma_material = Material::byNistName("Polymethyl Methacralate (Lucite, Perspex)").value();
    auto pmma_density = xraymc::NISTMaterials::density("Polymethyl Methacralate (Lucite, Perspex)");

    auto water_material = Material::byChemicalFormula("H2O").value();
    auto water_density = 1.0;

    item.setMaterial(aluminum_material, aluminum_density);

    // After we are done editing materials
    world.build(1.0);

    Beam beam;
    beam.setPosition({0, 0, 10});
    beam.setDirection({0, 0, -1});
    beam.setEnergy(70); // keV
    beam.setAirKerma(1.0);
    beam.setNumberOfExposures(100);
    beam.setNumberOfParticlesPerExposure(1E6);

    xraymc::Transport transport;
    // transport.setNumberOfThreads(20);
    transport.runConsole(world, beam);

    std::cout << "Depth dose in an aluminum cylinder of lenght " << item.length() << " cm";
    std::cout << ", radius of " << item.radius() << " cm\n";
    std::cout << "For a pencil beam with " << beam.energy() << " keV x-rays\n";
    std::cout << "Mass att. coeff for al is " << aluminum_material.attenuationValues(beam.energy()).sum();
    std::cout << " and density is " << aluminum_density << "g/cm3\n";

    double max_dose = 0;
    std::cout << "\nDepth [cm], Dose [keV], NumberOfEvents, Relative uncertanty [%]\n";
    for (const auto [depth, dose] : item.depthDoseScored())
    {
        std::cout << depth << ", " << dose.dose();
        std::cout << ", " << dose.numberOfEvents();
        std::cout << ", " << dose.relativeUncertainty() << std::endl;
        max_dose = std::max(max_dose, dose.dose());
    }

    xraymc::VisualizeWorld viz(world);
    viz.setDistance(300);
    viz.setAzimuthalAngleDeg(60);
    viz.suggestFOV(1);
    auto buffer = viz.createBuffer(1024, 1024);
    viz.addLineProp(beam.position(), beam.direction(), 10, 0.1);
    viz.generate(world, buffer);
    viz.savePNG("pencilbeam.png", buffer);

    viz.addColorByValueItem(world.getItemPointers()[0]);
    viz.setColorByValueMinMax(0, max_dose);
    viz.generate(world, buffer);
    viz.savePNG("pencilbeam_color.png", buffer);
    return EXIT_SUCCESS;
}