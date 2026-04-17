
// Example of a simple usecase of xraymc
// This example runs a Monte Carlo simulation of a
// pencil beam directed onto a cylinder to obtain depth-dose data

// Include xraymc header files
#include "xraymc/xraymc.hpp"

#include <iostream>

int main()
{
    // xraymc is a template library. Type aliases are defined here to simplify
    // usage — compile-time parameters (e.g. scoring type) are baked into each alias.
    using Beam = xraymc::PencilBeam<>;
    using DepthDose = xraymc::DepthDose<>;
    using World = xraymc::World<DepthDose>; // World is parameterized by the scorer type
    using Material = xraymc::Material<>;

    // Create the simulation world and reserve space for one geometry item
    // World items are geometry objects that can also accumulate scores (e.g. dose) during transport.
    // The world is filled with default air material at standard density.
    World world;
    world.reserveNumberOfItems(1);

    // Add a cylindrical depth-dose scorer to the world.
    // The item is the geometry object that also accumulates dose vs. depth.
    auto &item = world.addItem<DepthDose>();

    item.setRadius(2);             // cylinder radius in cm
    item.setDirection({0, 0, -1}); // cylinder axis points in the -z direction
    item.setLenght(10);            // cylinder length in cm
    item.setResolution(100);       // number of depth bins along the cylinder axis

    // --- Material definitions ---
    // Materials can be constructed from atomic number (Z), NIST name, or chemical formula.

    // Aluminum by atomic number (Z=13)
    auto aluminum_material = Material::byZ(13).value();
    auto aluminum_density = xraymc::AtomHandler::Atom(13).standardDensity; // g/cm³

    // PMMA (acrylic) by NIST name
    auto pmma_material = Material::byNistName("Polymethyl Methacralate (Lucite, Perspex)").value();
    auto pmma_density = xraymc::NISTMaterials::density("Polymethyl Methacralate (Lucite, Perspex)");
    // To get a list of all available NIST materials, use `xraymc::NISTMaterials::listNames()`, which returns a vector of strings.

    // Water by chemical formula
    auto water_material = Material::byChemicalFormula("H2O").value();
    auto water_density = 1.0; // g/cm³

    // Assign aluminum as the cylinder material
    item.setMaterial(aluminum_material, aluminum_density);

    // Build the world geometry. The argument (1.0) sets the world padding size in cm.
    // This must be called after all items and materials have been configured.
    world.build(1.0);

    // --- Beam configuration ---
    // A pencil beam is a monoenergetic, infinitely narrow photon beam.
    Beam beam;
    beam.setPosition({0, 0, 10});              // source position in cm (above the cylinder)
    beam.setDirection({0, 0, -1});             // directed in the -z direction toward the cylinder
    beam.setEnergy(70);                        // photon energy in keV
    beam.setAirKerma(1.0);                     // beam weight in terms of air kerma (mGy)
    beam.setNumberOfExposures(100);            // number of independent runs (for multithreading)
    beam.setNumberOfParticlesPerExposure(1E6); // photon histories per exposure

    // --- Run the Monte Carlo transport ---
    xraymc::Transport transport;
    transport.runConsole(world, beam); // runs simulation and prints progress to the console

    // --- Print summary and scored depth-dose data ---
    std::cout << "Depth dose in an aluminum cylinder of lenght " << item.length() << " cm";
    std::cout << ", radius of " << item.radius() << " cm\n";
    std::cout << "For a pencil beam with " << beam.energy() << " keV x-rays\n";
    std::cout << "Mass att. coeff for al is " << aluminum_material.attenuationValues(beam.energy()).sum();
    std::cout << " and density is " << aluminum_density << "g/cm3\n";

    // Iterate over depth-dose bins and print depth, dose, event count, and relative uncertainty
    double max_dose = 0;
    std::cout << "\nDepth [cm], Dose [keV], NumberOfEvents, Relative uncertanty [%]\n";
    for (const auto [depth, dose] : item.depthDoseScored())
    {
        std::cout << depth << ", " << dose.dose();
        std::cout << ", " << dose.numberOfEvents();
        std::cout << ", " << dose.relativeUncertainty() << std::endl;
        max_dose = std::max(max_dose, dose.dose());
    }

    // --- Visualization ---
    // Render the world geometry and beam as a PNG image.
    xraymc::VisualizeWorld viz(world);
    viz.setDistance(300);                       // camera distance from the scene center
    viz.setAzimuthalAngleDeg(60);               // camera azimuthal angle in degrees
    viz.suggestFOV(1);                          // automatically suggest a field of view
    auto buffer = viz.createBuffer(1024, 1024); // allocate a 1024×1024 pixel render buffer

    // Draw the pencil beam as a line and render the plain geometry
    viz.addLineProp(beam.position(), beam.direction(), 10, 0.1);
    viz.generate(world, buffer);
    viz.savePNG("pencilbeam.png", buffer);

    // Re-render with the cylinder colored by deposited dose value
    viz.addColorByValueItem(world.getItemPointers()[0]);
    viz.setColorByValueMinMax(0, max_dose); // map color scale from 0 to peak dose
    viz.generate(world, buffer);
    viz.savePNG("pencilbeam_color.png", buffer);

    return EXIT_SUCCESS;
}
