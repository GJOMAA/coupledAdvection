    Info<< "Reading field psi0\n" << endl;
    Info<< "Reading field psi\n" << endl;
    volScalarField psi
    (
        IOobject
        (
            "psi",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    ); 

    volScalarField psi0
    (
        IOobject
        (
            "psi0",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        psi//mesh,
        //dimensionedScalar("psi0",dimless, 0.0)
    );

    Info<< "Reading field delta\n" << endl;
    volScalarField delta
    (
        IOobject
        (
            "delta",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("delta",dimless, 0.0)
    );

    Info<< "Reading field H\n" << endl;
    volScalarField H
    (
        IOobject
        (
            "H",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("H",dimless, 0.0)
    );
    H.oldTime();

    volScalarField C
    (
        IOobject
        (
            "C",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensionedScalar("C",dimless/dimLength, 0.0)
    );

#include "createRDeltaT.H"

Info<< "Reading field p_rgh\n" << endl;
volScalarField p_rgh
(
    IOobject
    (
        "p_rgh",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);

Info<< "Reading field U\n" << endl;
volVectorField U
(
    IOobject
    (
        "U",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);

#include "createPhi.H"


Info<< "Reading transportProperties\n" << endl;
immiscibleIncompressibleTwoPhaseMixture mixture(U, phi);

volScalarField& alpha1(mixture.alpha1());
volScalarField& alpha2(mixture.alpha2());

const dimensionedScalar& rho1 = mixture.rho1();
const dimensionedScalar& rho2 = mixture.rho2();


// Need to store rho for ddt(rho, U)
volScalarField rho
(
    IOobject
    (
        "rho",
        runTime.timeName(),
        mesh,
        IOobject::READ_IF_PRESENT
    ),
    alpha1*rho1 + alpha2*rho2
);
rho.oldTime();


// Mass flux
surfaceScalarField rhoPhi
(
    IOobject
    (
        "rhoPhi",
        runTime.timeName(),
        mesh,
        IOobject::NO_READ,
        IOobject::NO_WRITE
    ),
    fvc::interpolate(rho)*phi
);


// Construct incompressible turbulence model
autoPtr<incompressible::turbulenceModel> turbulence
(
    incompressible::turbulenceModel::New(U, phi, mixture)
);


#include "readGravitationalAcceleration.H"
#include "readhRef.H"
#include "gh.H"


volScalarField p
(
    IOobject
    (
        "p",
        runTime.timeName(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    p_rgh + rho*gh
);

label pRefCell = 0;
scalar pRefValue = 0.0;
setRefCell
(
    p,
    p_rgh,
    pimple.dict(),
    pRefCell,
    pRefValue
);

if (p_rgh.needReference())
{
    p += dimensionedScalar
    (
        "p",
        p.dimensions(),
        pRefValue - getRefCellValue(p, pRefCell)
    );
    p_rgh = p - rho*gh;
}

mesh.setFluxRequired(p_rgh.name());
mesh.setFluxRequired(alpha1.name());

#include "createMRF.H"
#include "createIsoAdvection.H"

    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
        )
    );

dictionary LevelSetParameter
(
    transportProperties.subDict("LevelSetParameter")
);

const scalar ENUM
(
    readScalar(LevelSetParameter.lookup("interface_thickness"))
);

const scalar DX
(
    2.0/(max(mesh.deltaCoeffs()).value())
);

dimensionedScalar gamma
(
    dimensionedScalar(DX*0.75)
);

const scalar epsilon(ENUM*DX);

const scalar CFL
(
    readScalar(LevelSetParameter.lookup("cfl_number"))
);

dimensionedScalar sigma
(
    transportProperties.lookup("sigma")
);

const dimensionedScalar deltaN_
(
    "deltaN_",
    1e-10/pow(average(mesh.V()),1.0/3.0)
);

word phase1Name
(
    wordList(transportProperties.lookup("phases"))[0]
);

word phase2Name
(
    wordList(transportProperties.lookup("phases"))[1]
);


dimensionedScalar nu1
(
    transportProperties.subDict(phase1Name).lookup("nu")
);

dimensionedScalar nu2
(
    transportProperties.subDict(phase2Name).lookup("nu")
);
