
 R__LOAD_LIBRARY(libfun4all_universal_output_manager)
R__LOAD_LIBRARY(libcalibrator)

int run(const char* fn_udst="/data2/e1039/dst/run_006178/run_006178_spill_001951171_spin.root") {
    Fun4AllServer* se = Fun4AllServer::instance();
    // se->Verbosity(1);  // Optionally set verbosity level if needed
    Fun4AllInputManager *in = new Fun4AllDstInputManager("DUMMY");
    se->registerInputManager(in);
    in->fileopen(fn_udst);
   
    CalibDriftDist* cal_dd = new CalibDriftDist();
    se->registerSubsystem(cal_dd);

    Fun4AllUniversalOutputManager* tree = new Fun4AllUniversalOutputManager();
    //tree->SetFileName("data/out_kLZMA_1_64kb__flush4000_Lin.root");
    tree->SetFileName("test.root");
    tree->SetTreeName("tree");
    se->registerOutputManager(tree);

    se->run();
    se->End();

    // Do not delete se; // Removing manual deletion of singleton

    return 0;
}

