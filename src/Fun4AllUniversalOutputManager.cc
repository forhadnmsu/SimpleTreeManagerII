#include <cstdlib>
#include <string>
#include <iostream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <phool/phool.h>
#include <phool/getClass.h>
#include <phool/PHNode.h>
#include <phool/PHNodeIOManager.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQSpillMap.h>
#include <interface_main/SQHitVector.h>
#include <db_svc/DbSvc.h>
#include "Fun4AllUniversalOutputManager.h"
using namespace std;

Fun4AllUniversalOutputManager::Fun4AllUniversalOutputManager(const std::string &myname)
  : Fun4AllOutputManager(myname),
    m_file(0),
    m_tree(0),
    m_tree_name("tree"),
    m_file_name("output.root"),
    m_evt(0),
    m_sp_map(0),
    m_hit_vec(0)
{
 ;
}

Fun4AllUniversalOutputManager::~Fun4AllUniversalOutputManager() {
    CloseFile();
}

void Fun4AllUniversalOutputManager::OpenFile() {
std::cout << "Fun4AllUniversalOutputManager::OpenFile(): Attempting to open file: " << m_file_name << " with tree: " << m_tree_name << std::endl;
m_file = new TFile(m_file_name.c_str(), "RECREATE");

if (!m_file || m_file->IsZombie()) {
    std::cerr << "Error: Could not create file " << m_file_name << std::endl;
    exit(1);
} else {
    std::cout << "File " << m_file->GetName() << " opened successfully." << std::endl;
}

timer.Start();
m_file->SetCompressionAlgorithm(ROOT::kLZMA);
m_file->SetCompressionLevel(1);
m_tree = new TTree(m_tree_name.c_str(), "Tree for storing events");
if (!m_tree) {
    std::cerr << "Error: Could not create tree " << m_tree_name << std::endl;
    exit(1);
} else {
    std::cout << "Tree " << m_tree->GetName() << " created successfully." << std::endl;
}

    m_tree->Branch("RunID", &RunID, "RunID/I");
    m_tree->Branch("SpillID", &SpillID, "SpillID/I");
    m_tree->Branch("EventID", &EventID, "EventID/I");
    m_tree->Branch("RFID", &RFID, "RFID/I");
    m_tree->Branch("TurnID", &TurnID, "TurnID/I");
    m_tree->Branch("Intensity", Intensity, "Intensity[33]/I");
    m_tree->Branch("trig_bits", &trig_bits);
    m_tree->Branch("detectorID", &detectorID);
    m_tree->Branch("elementID", &elementID);
    m_tree->Branch("tdcTime", &tdcTime);
    m_tree->Branch("driftDistance", &driftDistance);
    m_tree->SetBasketSize("*", 64000);  // 64 KB

}
int Fun4AllUniversalOutputManager::Write(PHCompositeNode* startNode) {
    if (!m_file || !m_tree) {
        OpenFile();
    }

    if (!m_evt) {
        m_evt = findNode::getClass<SQEvent>(startNode, "SQEvent");
        m_hit_vec = findNode::getClass<SQHitVector>(startNode, "SQHitVector");

        if (!m_evt) {
            cout << PHWHERE << "Cannot find the SQ data nodes. Abort." << endl;
            exit(1);
        }
    }

    RunID = m_evt->get_run_id();
    SpillID = m_evt->get_spill_id();
    RFID = m_evt->get_qie_rf_id();
    EventID = m_evt->get_event_id();
    TurnID = m_evt->get_qie_turn_id();
    trig_bits = m_evt->get_trigger();

    detectorID.clear();
    elementID.clear();
    tdcTime.clear();
    driftDistance.clear();

    for (int i = -16; i <= 16; ++i) {
        Intensity[i+16] = m_evt->get_qie_rf_intensity(i);
    }

    if (m_hit_vec) {
        for (int ihit = 0; ihit < m_hit_vec->size(); ++ihit) {
            SQHit* hit = m_hit_vec->at(ihit);
            detectorID.push_back(hit->get_detector_id());
            elementID.push_back(hit->get_element_id());
            tdcTime.push_back(hit->get_tdc_time());
            driftDistance.push_back(hit->get_drift_distance());
        }
    }

    m_tree->Fill();
    return 0;
}


void Fun4AllUniversalOutputManager::CloseFile() {
    if (!m_file) return;
    std::cout << "Fun4AllUniversalOutputManager::CloseFile(): Closing file: " << m_file_name << std::endl;
    m_file->Write();
    m_file->Close();
    timer.Stop();
    double compressionTime = timer.RealTime();
    std::cout << "Compression time: " << compressionTime << " seconds" << std::endl;
    delete m_file;
    m_file = nullptr;
}
