#include <TFile.h>
#include <TTree.h>
#include <TLorentzVector.h>
#include <vector>
#include <iostream>

void s() {
    // Abrir el archivo ROOT de entrada
    TFile *file = TFile::Open("myoutput17.root");
    if (!file) {
        std::cerr << "Error al abrir el archivo" << std::endl;
        return;
    }

    // Obtener los árboles
    TTree *tree_electron = (TTree*)file->Get("myelectrons/Events");
    if (!tree_electron) {
        std::cerr << "Error al cargar el árbol de electrones." << std::endl;
        return;
    }
    TTree *tree_met = (TTree*)file->Get("mymets/Events");
    if (!tree_met) {
        std::cerr << "Error al cargar el árbol de MET." << std::endl;
        return;
    }

    // Definir las variables para los electrones y MET
    std::vector<float> *pt_electron = nullptr;
    std::vector<float> *phi_electron = nullptr;
    float pt_met = 0;
    float phi_met = 0;

    // Configuración de las ramas
    tree_electron->SetBranchAddress("electron_pt", &pt_electron);
    tree_electron->SetBranchAddress("electron_phi", &phi_electron);
    tree_met->SetBranchAddress("met_pt", &pt_met);
    tree_met->SetBranchAddress("met_phi", &phi_met);

    // Crear el archivo ROOT de salida
    TFile *output_file = new TFile("output_separated_trees.root", "RECREATE");

    // Crear dos árboles, uno para fondo y otro para señal
    TTree *tree_signal = new TTree("signal_tree", "Árbol de la señal");
    TTree *tree_background = new TTree("background_tree", "Árbol del fondo");

    // Variables para las ramas de salida
    float pt_met_out = 0;
    float pt_electron_out = 0;
    float phi_electron_out = 0;
    float phi_met_out = 0;

    // Crear ramas para señal y fondo
    tree_signal->Branch("pt_electron", &pt_electron_out, "pt_electron/F");
    tree_signal->Branch("phi_electron", &phi_electron_out, "phi_electron/F");
    tree_signal->Branch("phi_met", &phi_met_out, "phi_met/F");
    tree_signal->Branch("pt_met", &pt_met_out, "pt_met/F");

    tree_background->Branch("pt_electron", &pt_electron_out, "pt_electron/F");
    tree_background->Branch("phi_electron", &phi_electron_out, "phi_electron/F");
    tree_background->Branch("phi_met", &phi_met_out, "phi_met/F");
    tree_background->Branch("pt_met", &pt_met_out, "pt_met/F");

    // Procesar los eventos
    Long64_t nEntries = tree_electron->GetEntries();
    for (Long64_t i = 0; i < nEntries; ++i) {
        tree_electron->GetEntry(i);
        tree_met->GetEntry(i);

        // Recorrer los electrones en el evento
        for (size_t k = 0; k < pt_electron->size(); ++k) {
            pt_electron_out = pt_electron->at(k);
            phi_electron_out = phi_electron->at(k);
            pt_met_out = pt_met;
            phi_met_out = phi_met;

            // Clasificar en señal o fondo
            if (pt_electron_out > 20 && pt_electron_out < 130) { // Ejemplo de criterio para señal
                tree_signal->Fill();
            } else {
                tree_background->Fill();
            }
        }
    }

    // Guardar los árboles en el archivo de salida
    tree_signal->Write();
    tree_background->Write();

    // Cerrar los archivos
    output_file->Close();
    file->Close();
}
