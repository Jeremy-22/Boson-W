#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <cmath>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TRandom3.h>
#include <TMath.h>


Double_t breitWigner(Double_t *x, Double_t *par) {
    Double_t E = x[0];
    Double_t E0 = par[0];  // Pico de Breit-Wigner
    Double_t Gamma = par[1];  // Ancho de Breit-Wigner
    Double_t A = par[2];  // Amplitud
    return (A / (2 * TMath::Pi())) * (Gamma / ((E - E0)*(E - E0) + (Gamma / 2)*(Gamma / 2)));
}
Double_t crystalBall(Double_t *x, Double_t *par) {
    Double_t E = x[0];
    Double_t alpha = par[0];
    Double_t n = par[1];
    Double_t mu = par[2];  // Pico de Crystal Ball
    Double_t sigma = par[3];  // Ancho de Crystal Ball
    Double_t A = par[4];  // Amplitud

    if ((E - mu) / sigma > -alpha) {
        return A * exp(-0.5 * pow((E - mu) / sigma, 2));
    } else {
        Double_t A1 = pow(n / fabs(alpha), n) * exp(-0.5 * alpha * alpha);
        Double_t B1 = n / fabs(alpha) - fabs(alpha);
        return A * A1 * pow(B1 - (E - mu) / sigma, -n);
    }
}
//Adicion de BreitWigner y CrystallBall
Double_t breitWignerCrystalBall(Double_t *x, Double_t *par) {
    return breitWigner(x, &par[0]) + crystalBall(x, &par[3]);
}

void fit() {


    TFile *file = TFile::Open("myoutput18.root");
    if (!file || file->IsZombie()) {
        std::cerr << "Error al abrir el archivo ROOT." << std::endl;
        return;
    }

    TTree *tree_electron = (TTree*)file->Get("myelectrons/Events");
    if (!tree_electron) {
        std::cerr << "Error al cargar el árbol de electrones." << std::endl;
        return;
    }

    TTree *tree_mets = (TTree*)file->Get("mymets/Events");
    if (!tree_mets) {
        std::cerr << "Error al cargar el árbol de MET." << std::endl;
        return;
    }

    std::vector<float> *electron_pt = nullptr;
    std::vector<float> *electron_phi = nullptr;
    float met_pt = 0;
    float met_phi = 0;

    tree_electron->SetBranchAddress("electron_pt", &electron_pt);
    tree_electron->SetBranchAddress("electron_phi", &electron_phi);
    tree_mets->SetBranchAddress("met_pt", &met_pt);
    tree_mets->SetBranchAddress("met_phi", &met_phi);

    TH1F *h_mt = new TH1F("h_mt", "Masa Transversa del W;M_{T} (GeV/c^{2});Eventos", 100, 0, 200);
    Long64_t nEntries = tree_electron->GetEntries();
    for (Long64_t i = 0; i < nEntries; ++i) {
        tree_electron->GetEntry(i);
        tree_mets->GetEntry(i);
        if (electron_pt->size() > 0) {
            float pt_electron = electron_pt->at(0);
            float phi_electron = electron_phi->at(0); 
            float delta_phi = TMath::Abs(phi_electron - met_phi);
            if (delta_phi > TMath::Pi()) delta_phi = 2 * TMath::Pi() - delta_phi;
            float mt = TMath::Sqrt(2 * pt_electron * met_pt * (1 - TMath::Cos(delta_phi)));

            h_mt->Fill(mt);
        }
    }
    TF1 *fitFunction = new TF1("fitFunction", breitWignerCrystalBall, 0, 200, 8);
   //fitFunction->SetParameters(80, 15, h_mt->GetMaximum(), 1.5, 2, 90, 15, h_mt->GetMaximum());
   //fitFunction->SetParameters(80, 15, h_mt->GetMaximum(), 1.7, 2, 90, 15, h_mt->GetMaximum());
    //fitFunction->SetParameters(55, 9, h_mt->GetMaximum(), 1.5, 5, 60, 2, h_mt->GetMaximum());
    fitFunction->SetParameters(75, 15, h_mt->GetMaximum(), 1.5, 1.2, 85, 15, h_mt->GetMaximum());//era 1.5 y 1.2
    //fitFunction->SetParameters(60, 15, h_mt->GetMaximum(), 1.9, 1.9, 60, 15, h_mt->GetMaximum());
    h_mt->Fit(fitFunction, "R");

    TCanvas *canvas = new TCanvas("canvas", "Ajuste Breit-Wigner + Crystal Ball", 800, 600);
    h_mt->Draw();
    fitFunction->Draw("same");  
    TLegend *legend = new TLegend(0.7, 0.8, 0.9, 0.9);
    legend->AddEntry(h_mt, "Datos", "l");
    legend->AddEntry(fitFunction, "Ajuste Breit-Wigner + Crystal Ball", "l");
    legend->Draw();
    
    TFile *output = new TFile("out.root", "RECREATE");
    output->cd();
    h_mt->Write();
    fitFunction->Write();
    canvas->Write();
    canvas->SaveAs("fit_result7.png");
    output->Close();
    delete canvas;
    delete file;
    delete output;
    std::cout << "guardado en graph/out.root." << std::endl;
    return;
}
