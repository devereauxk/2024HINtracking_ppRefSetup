#include "RiceStyle.h"
#include "plotting.h"

void plotHist2D_byPdgId() {

  RiceStyle();

  gStyle->SetOptStat(0);

  //TFile *f = new TFile("/afs/cern.ch/user/k/kdeverea/CMSSW_14_1_7/src/HITrackingStudies/HITrackingStudies/test/LowPU_Pythia_5p36TeV_pdgidtest.root");
  TFile *f = new TFile("/afs/cern.ch/user/k/kdeverea/CMSSW_15_0_8/src/HITrackingStudies/HITrackingStudies/test/OfficialOOMC_Hijing_5p36TeV_byPdgId.root");
  std::vector<int> pdgIdSelections = {0, 2212, 211, 321, 3222, 3112, 9999}; // 0 is for all particles, 9999 is for particles not specified

  const char *prefix = "AbsoluteEfficiency_OOOfficialHijing";

  TFile *fweights = new TFile(Form("files_byPdgId/%s_byPdgId.root", prefix), "RECREATE");

  char ndir[256] = "HITrackCorrections_byPdgId";
  double ptmax = 120.;

  vector<TGraphAsymmErrors*> gEffEtas;
  vector<TGraphAsymmErrors*> gEffPts;
  vector<TH1D*> hEff_1Ds;
  for (size_t i = 0; i < pdgIdSelections.size(); ++i) {
    int pdgId = pdgIdSelections[i];
    TString pdgStr = Form("%d", pdgId);
    TString suffix = pdgStr;

    // Load histograms for this pdgId
    TString hSimName = Form("%s/hsim_%s", ndir, pdgStr.Data());
    TString hEffName = Form("%s/heff_%s", ndir, pdgStr.Data());

    TH2F *hSim = (TH2F*) f->Get(hSimName); 
    TH2F *hEff = (TH2F*) f->Get(hEffName); 
    if (!hSim || !hEff) {
      printf("Warning: Could not find histograms for pdgId %d\n", pdgId);
      continue;
    }
    hSim->GetYaxis()->SetRangeUser(0.2,ptmax);
    hEff->GetYaxis()->SetRangeUser(0.2,ptmax);

    // ratio histogram
    TH2F *rEff = (TH2F*) hEff->Clone(Form("rEff_%s", pdgStr.Data()));
    rEff->Divide(hEff,hSim,1,1,"B");
    rEff->SetStats(0);
    rEff->SetMaximum(1.0); rEff->SetMinimum(0.0);
    rEff->SetTitle(Form("Absolute Efficiency pdgId=%d", pdgId));

    rEff->Write();

    // Find bins for projections
    Int_t ptbin04=hSim->GetYaxis()->FindBin(0.41);
    Int_t ptbin20=hSim->GetYaxis()->FindBin(2.01);
    Int_t ptbins=hSim->GetYaxis()->GetNbins();
    Int_t etabin24m=hSim->GetXaxis()->FindBin(-2.39);
    Int_t etabin24p=hSim->GetXaxis()->FindBin(2.39);
    Int_t etabin10m=hSim->GetXaxis()->FindBin(-0.99);
    Int_t etabin10p=hSim->GetXaxis()->FindBin(0.99);

    // projected hists: pt > 0.4 GeV/c
    TH1D* hSimEta = (TH1D*) hSim->ProjectionX(Form("hSimEta_%s", pdgStr.Data()),ptbin04,ptbins,"e");
    TH1D* hEffEta = (TH1D*) hEff->ProjectionX(Form("hEffEta_%s", pdgStr.Data()),ptbin04,ptbins,"e");

    // projected hists: pt > 2.0 GeV/c
    TH1D* hSimEta2 = (TH1D*) hSim->ProjectionX(Form("hSimEta2_%s", pdgStr.Data()),ptbin20,ptbins,"e");
    TH1D* hEffEta2 = (TH1D*) hEff->ProjectionX(Form("hEffEta2_%s", pdgStr.Data()),ptbin20,ptbins,"e");

    TH1D* hDumEta = new TH1D(Form("hDumEta_%s", pdgStr.Data()),";#eta",60,-2.4,2.4); hDumEta->SetMaximum(1.0);
    hDumEta->GetXaxis()->CenterTitle(); hDumEta->GetYaxis()->SetTitleOffset(1.8);

    // projected hists: abs(eta) < 1.0
    TH1D* hSimPt  = (TH1D*) hSim->ProjectionY(Form("hSimPt_%s", pdgStr.Data()),etabin10m,etabin10p,"e");
    TH1D* hEffPt  = (TH1D*) hEff->ProjectionY(Form("hEffPt_%s", pdgStr.Data()),etabin10m,etabin10p,"e");

    // projected hists: abs(eta) < 2.4
    TH1D* hSimPt2  = (TH1D*) hSim->ProjectionY(Form("hSimPt2_%s", pdgStr.Data()),etabin24m,etabin24p,"e");
    TH1D* hEffPt2  = (TH1D*) hEff->ProjectionY(Form("hEffPt2_%s", pdgStr.Data()),etabin24m,etabin24p,"e");

    TH1D* hDumPt = new TH1D(Form("hDumPt_%s", pdgStr.Data()),";p_{T} [GeV/c]",80,0.1,120.0); hDumPt->SetMaximum(1.0);
    hDumPt->GetXaxis()->CenterTitle(); hDumPt->GetYaxis()->SetTitleOffset(1.8);

    // Efficiency
    TGraphAsymmErrors *gEffEta = new TGraphAsymmErrors(); gEffEta->SetName(Form("gEffEta_%s", pdgStr.Data()));
    gEffEta->BayesDivide(hEffEta,hSimEta);
    gEffEta->SetMarkerStyle(25);
    gEffEta->SetLineStyle(2);
    gEffEta->SetLineColor(2);
    gEffEta->SetMarkerColor(2);

    gEffEtas.push_back(gEffEta);

    TGraphAsymmErrors *gEffPt = new TGraphAsymmErrors(); gEffPt->SetName(Form("gEffPt_%s", pdgStr.Data()));
    gEffPt->BayesDivide(hEffPt,hSimPt);
    gEffPt->SetMarkerStyle(24);
    gEffPt->SetLineColor(4);
    gEffPt->SetMarkerColor(4);

    gEffPts.push_back(gEffPt);

    // Get pT bin edges from the original 2D histogram
    TAxis* ptAxis = hSim->GetYaxis();
    int nPtBins = ptAxis->GetNbins();
    double* ptBinEdges = new double[nPtBins + 1];
    for (int i = 0; i <= nPtBins; i++) {
        ptBinEdges[i] = ptAxis->GetBinLowEdge(i + 1);
    }
    ptBinEdges[nPtBins] = ptAxis->GetBinUpEdge(nPtBins);

    // Create histogram with exact same binning as original
    TH1D* hEffPtIntegrated = new TH1D(Form("rEff_1D_%s", pdgStr.Data()), 
                                      Form("Integrated Eta Efficiency vs pT (pdgId=%d)", pdgId),
                                      nPtBins, ptBinEdges);

    // Direct calculation from original histograms (more accurate)
    for (int ipt = 1; ipt <= nPtBins; ipt++) {
        double simSum = 0, effSum = 0;
        
        // Sum over eta bins for this pT bin
        for (int ieta = etabin10m; ieta <= etabin10p; ieta++) {
            simSum += hSim->GetBinContent(ieta, ipt);
            effSum += hEff->GetBinContent(ieta, ipt);
        }
        
        double efficiency = (simSum > 0) ? effSum / simSum : 0;
        double error = (simSum > 0) ? sqrt(efficiency * (1 - efficiency) / simSum) : 0;
        
        hEffPtIntegrated->SetBinContent(ipt, efficiency);
        hEffPtIntegrated->SetBinError(ipt, error);
    }
    hEffPtIntegrated->Write();
    hEff_1Ds.push_back(hEffPtIntegrated);

    TGraphAsymmErrors *gEffEta2 = new TGraphAsymmErrors(); gEffEta2->SetName(Form("gEffEta2_%s", pdgStr.Data()));
    gEffEta2->BayesDivide(hEffEta2,hSimEta2);
    gEffEta2->SetMarkerStyle(24);
    gEffEta2->SetLineColor(4);
    gEffEta2->SetMarkerColor(4);

    TGraphAsymmErrors *gEffPt2 = new TGraphAsymmErrors(); gEffPt2->SetName(Form("gEffPt2_%s", pdgStr.Data()));
    gEffPt2->BayesDivide(hEffPt2,hSimPt2);
    gEffPt2->SetMarkerStyle(25);
    gEffPt2->SetLineStyle(2);
    gEffPt2->SetLineColor(2);
    gEffPt2->SetMarkerColor(2);

    TLegend *legEta = new TLegend(0.35,0.15,0.65,0.30);
    legEta->SetFillColor(0); legEta->SetBorderSize(0);
    legEta->AddEntry(gEffEta,"p_{T} > 0.4 GeV/c","lp");
    legEta->AddEntry(gEffEta2,"p_{T} > 2.0 GeV/c","lp");

    TLegend *legPt = new TLegend(0.4,0.2,0.6,0.35);
    legPt->SetFillColor(0); legPt->SetBorderSize(0);
    legPt->AddEntry(gEffPt2,"|#eta| < 2.4","lp");
    legPt->AddEntry(gEffPt,"|#eta| < 1.0","lp");

    //Dener--------------------------------
    TH1F* hDumEtaEff;
    TH1F* hDumPtEff;
    //-------------------------------------
    TCanvas* c7 = makeMultiCanvas(Form("c7_%s", pdgStr.Data()), "Efficiency Fraction", 2,1 );
    hDumEtaEff=(TH1F*) hDumEta->Clone(Form("hDumEtaEff_%s", pdgStr.Data()));
    fixedFontHist1D(hDumEtaEff,1.05,1.2);
    hDumEtaEff->GetYaxis()->SetTitle("Absolute efficiency");
    hDumPtEff=(TH1F*) hDumPt->Clone(Form("hDumPtEff_%s", pdgStr.Data()));
    fixedFontHist1D(hDumPtEff,1.05,1.2);
    hDumPtEff->GetYaxis()->SetTitle("Absolute efficiency");

    c7->cd(1); gPad->SetTicks(); c7->GetPad(1)->SetLeftMargin(0.12); c7->GetPad(1)->SetBottomMargin(0.13); c7->GetPad(1)->SetLogx(0); hDumEtaEff->Draw(); gEffEta->Draw("pc"); gEffEta2->Draw("pc"); legEta->Draw();

    c7->cd(2); gPad->SetTicks(); c7->GetPad(2)->SetLeftMargin(0.12); c7->GetPad(2)->SetBottomMargin(0.13); c7->GetPad(2)->SetLogx(); hDumPtEff->Draw(); gEffPt->Draw("pc"); gEffPt2->Draw("pc"); legPt->Draw();
    saveCanvas(c7, "files_byPdgId", Form("%s_%s", prefix, suffix.Data()));

  }

  // =================================================================
  // Overlay particle species
  // =================================================================

  // Overlay all species: plot all gEffEtas and gEffPts on the same canvas, with different colors

  // Define colors and markers for each species
  std::vector<int> colors = {kBlack, kRed, kSpring, kBlue, kPink+1, kCyan+1, kGreen+2};
  std::vector<int> markers = {20, 21, 22, 23, 33, 34, 20};

  // Dummy histograms for axes
  TH1D* hDumEtaAll = new TH1D("hDumEtaAll",";#eta;Absolute efficiency",60,-2.4,2.4);
  hDumEtaAll->SetMaximum(1.5); hDumEtaAll->SetMinimum(0.0);
  hDumEtaAll->GetXaxis()->CenterTitle(); hDumEtaAll->GetYaxis()->SetTitleOffset(1.8);

  TH1D* hDumPtAll = new TH1D("hDumPtAll",";p_{T} [GeV/c];Absolute efficiency",80,0.1,120.0);
  hDumPtAll->SetMaximum(1.5); hDumPtAll->SetMinimum(0.0);
  hDumPtAll->GetXaxis()->CenterTitle(); hDumPtAll->GetYaxis()->SetTitleOffset(1.8);

  // Legend (top right)
  TLegend* legEtaAll = new TLegend(0.25,0.63,0.55,0.85);
  legEtaAll->SetFillColor(0); legEtaAll->SetBorderSize(0);
  legEtaAll->SetTextSize(0.037); // Increase font size
  TLegend* legPtAll = new TLegend(0.25,0.63,0.55,0.85);
  legPtAll->SetFillColor(0); legPtAll->SetBorderSize(0);
  legPtAll->SetTextSize(0.037); // Increase font size

  // pdgId labels for legend
  std::vector<TString> pdgLabels = {"Inclusive", "p + \\bar{p}", "#pi^{+} + #pi^{-}", "K^{+} + K^{-}", "#Sigma^{+} + \\bar{#Sigma}^{-}", "#Sigma^{-} + \\bar{#Sigma}^{+}", "remainders"};

  // Canvas
  TCanvas* cAll = makeMultiCanvas("cAll_species", "Efficiency by Species", 2, 1);

  // Draw eta efficiency
  cAll->cd(1); gPad->SetTicks(); cAll->GetPad(1)->SetLeftMargin(0.12); cAll->GetPad(1)->SetBottomMargin(0.13); cAll->GetPad(1)->SetLogx(0);
  hDumEtaAll->Draw();
  for (size_t i = 0; i < gEffEtas.size(); ++i) {
    gEffEtas[i]->SetLineColor(colors[i % colors.size()]);
    gEffEtas[i]->SetMarkerColor(colors[i % colors.size()]);
    gEffEtas[i]->SetMarkerStyle(markers[i % markers.size()]);
    gEffEtas[i]->Draw("pc same");
    legEtaAll->AddEntry(gEffEtas[i], pdgLabels[i], "lp");
  }
  legEtaAll->Draw();

  // Draw pt efficiency
  cAll->cd(2); gPad->SetTicks(); cAll->GetPad(2)->SetLeftMargin(0.12); cAll->GetPad(2)->SetBottomMargin(0.13); cAll->GetPad(2)->SetLogx();
  hDumPtAll->Draw();
  for (size_t i = 0; i < gEffPts.size(); ++i) {
    gEffPts[i]->SetLineColor(colors[i % colors.size()]);
    gEffPts[i]->SetMarkerColor(colors[i % colors.size()]);
    gEffPts[i]->SetMarkerStyle(markers[i % markers.size()]);
    gEffPts[i]->Draw("pc same");
    legPtAll->AddEntry(gEffPts[i], pdgLabels[i], "lp");
  }
  legPtAll->Draw();

  saveCanvas(cAll, "files_byPdgId", Form("%s_AllSpecies", prefix));

  fweights->Close();

}



void set_plot_style() {
  // nicer colz plots
  const Int_t NRGBs = 5;
  const Int_t NCont = 255;

  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);
}
