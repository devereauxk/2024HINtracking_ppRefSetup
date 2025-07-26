#include "plotting.h"

// =================================================================
// Overlay 1D pt dependencies
// =================================================================

void plotHist1D() {

    TFile *f = new TFile("files_byPdgId/AbsoluteEfficiency_ppOfficialHijing_byPdgId.root");
    vector<int> pdgIdSelections = {0, 2212, 211, 321, 3222, 3112, 9999};
    vector<string> pdgLabels = {"Inclusive", "p + \\bar{p}", "#pi^{+} + #pi^{-}", "K^{+} + K^{-}", "#Sigma^{+} + \\bar{#Sigma}^{-}", "#Sigma^{-} + \\bar{#Sigma}^{+}", "remainders"};

    const char *prefix = "AbsoluteEfficiency_ppOfficialHijing";

    vector<TH1D*> hEff_1Ds;
    for (int i = 0; i<pdgIdSelections.size(); ++i) {
        int pdgId = pdgIdSelections[i];
        TString pdgStr = Form("%d", pdgId);
        TString suffix = pdgStr;

        // Load histograms for this pdgId
        TString hEffName = Form("rEff_1D_%s", pdgStr.Data());
        TH1D *hEff = (TH1D*) f->Get(hEffName);
        if (!hEff) {
            printf("Warning: Could not find histogram %s\n", hEffName.Data());
            continue;
        }
        hEff_1Ds.push_back(hEff);
    }


    // =================================================================
    // PLOT

    TCanvas *c1 = new TCanvas("c1", "Efficiency by Species", 800, 600);

    TPad* pad1 = (TPad*) plotCMSSimple(
        c1, hEff_1Ds, "", pdgLabels,
        {kBlack, kRed, kSpring, kBlue, kPink+1, kCyan+1, kGreen+2}, {0, 0, 0, 0, 0, 0, 0},
        {kBlack, kRed, kSpring, kBlue, kPink+1, kCyan+1, kGreen+2}, {20, 21, 22, 23, 33, 34, 20},
        "p_{T}", 0.1, 120.0,
        "Absolute efficiency", 0.0, 1.5,
        true, false, false
    );

    AddCMSHeader(
        pad1,      // Provide the TPad
        "Internal", // (optional) Add a subheader to the CMS header
        true
    );

    AddUPCHeader(pad1, "5.36 TeV", "pp Official Hijing");
    pad1->Update();

    c1->SaveAs(Form("files_byPdgId/%s_1D.pdf", prefix));

}
