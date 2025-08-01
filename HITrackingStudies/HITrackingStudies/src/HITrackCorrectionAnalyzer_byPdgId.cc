#include <memory>
#include <algorithm>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TF1.h>
#include <TTree.h>

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include <DataFormats/VertexReco/interface/Vertex.h>
#include <DataFormats/VertexReco/interface/VertexFwd.h>
#include <DataFormats/TrackReco/interface/Track.h>
#include <DataFormats/TrackReco/interface/TrackFwd.h>
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticleFwd.h"
#include "SimDataFormats/Associations/interface/TrackAssociation.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

// Particle Flow
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowReco/interface/PFBlock.h"
#include "DataFormats/ParticleFlowReco/interface/PFCluster.h"
#include "DataFormats/ParticleFlowReco/interface/PFClusterFwd.h"
// RecoJets
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/JetReco/interface/CaloJet.h"
#include "DataFormats/JetReco/interface/CaloJetCollection.h"

#include "HITrackingStudies/HITrackingStudies/interface/HITrackCorrectionTreeHelper.h"

class HITrackCorrectionAnalyzer_byPdgId : public edm::one::EDAnalyzer<edm::one::SharedResources> {
   public:
      explicit HITrackCorrectionAnalyzer_byPdgId(const edm::ParameterSet&);
      ~HITrackCorrectionAnalyzer_byPdgId();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions); 
      static bool vtxSort( const reco::Vertex &  a, const reco::Vertex & b );

   private:
      virtual void beginJob() ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      void initHistos(const edm::Service<TFileService> & fs);
      bool multCuts(const reco::Track & track, const reco::Vertex & vertex);
      bool passesTrackCuts(const reco::Track & track, const reco::Vertex & vertex);
      bool caloMatched(const reco::Track & track, const edm::Event& iEvent, unsigned it );

      // ----------member data ---------------------------


      std::map<std::string,TTree*> trkTree_;
      std::map<std::string,std::map<int, TH2F*>> trkCorr2D_;
      std::map<std::string,TH3F*> trkCorr3D_;
      TH3F * momRes_;
      TH1F * vtxZ_;
      TH1F * pthat_;
      TF1 * vtxWeightFunc_;

      std::vector<int> pdgIdSelections = {0, 2212, 211, 321, 3222, 3112, 9999}; // 0 is for all other particles, 9999 is for remainder
      TH1F * pdgId_;

      HITrackCorrectionTreeHelper treeHelper_;


      edm::EDGetTokenT<reco::VertexCollection> vertexSrc_;
      edm::EDGetTokenT<edm::View<reco::Track> > trackSrc_;
      edm::EDGetTokenT<TrackingParticleCollection> tpFakSrc_;
      edm::EDGetTokenT<TrackingParticleCollection> tpEffSrc_;
      edm::EDGetTokenT<reco::RecoToSimCollection> associatorMapRTS_;
      edm::EDGetTokenT<reco::SimToRecoCollection> associatorMapSTR_;      
      edm::EDGetTokenT<GenEventInfoProduct> genInfoToken_;
  
      std::vector<double> ptBins_;
      std::vector<double> etaBins_;
      std::vector<double> occBins_;

      bool doCaloMatched_;
      double reso_;
      double crossSection_;
      
      std::vector<double> vtxWeightParameters_;
      std::vector<int> algoParameters_;
      bool doVtxReweighting_;
      bool dopthatWeight_;
  
      bool applyVertexZCut_;
      double vertexZMax_;

      bool applyTrackCuts_;
      std::string qualityString_;
      double dxyErrMax_;
      double dzErrMax_;
      double ptErrMax_;
      int    nhitsMin_;
      double chi2nMax_;      

      bool doMomRes_;

      bool fillNTuples_;

      bool useCentrality_;
      edm::EDGetTokenT<int> centralitySrc_;
      edm::EDGetTokenT<reco::CaloJetCollection> jetSrc_;
      edm::EDGetTokenT<reco::PFCandidateCollection> pfCandSrc_;


};

HITrackCorrectionAnalyzer_byPdgId::HITrackCorrectionAnalyzer_byPdgId(const edm::ParameterSet& iConfig):
treeHelper_(),
vertexSrc_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexSrc"))),
trackSrc_(consumes<edm::View<reco::Track> >(iConfig.getParameter<edm::InputTag>("trackSrc"))),
tpFakSrc_(consumes<TrackingParticleCollection>(iConfig.getParameter<edm::InputTag>("tpFakSrc"))),
tpEffSrc_(consumes<TrackingParticleCollection>(iConfig.getParameter<edm::InputTag>("tpEffSrc"))),
associatorMapRTS_(consumes<reco::RecoToSimCollection>(iConfig.getParameter<edm::InputTag>("associatorMap"))),
associatorMapSTR_(consumes<reco::SimToRecoCollection>(iConfig.getParameter<edm::InputTag>("associatorMap"))),
genInfoToken_(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
ptBins_(iConfig.getParameter<std::vector<double> >("ptBins")),
etaBins_(iConfig.getParameter<std::vector<double> >("etaBins")),
occBins_(iConfig.getParameter<std::vector<double> >("occBins")),
doCaloMatched_(iConfig.getParameter<bool>("doCaloMatched")),
reso_(iConfig.getParameter<double>("reso")),
crossSection_(iConfig.getParameter<double>("crossSection")),
vtxWeightParameters_(iConfig.getParameter<std::vector<double> >("vtxWeightParameters")),
algoParameters_(iConfig.getParameter<std::vector<int> >("algoParameters")),
doVtxReweighting_(iConfig.getParameter<bool>("doVtxReweighting")),
dopthatWeight_(iConfig.getParameter<bool>("dopthatWeight")),
applyVertexZCut_(iConfig.getParameter<bool>("applyVertexZCut")),
vertexZMax_(iConfig.getParameter<double>("vertexZMax")),
applyTrackCuts_(iConfig.getParameter<bool>("applyTrackCuts")),
qualityString_(iConfig.getParameter<std::string>("qualityString")),
dxyErrMax_(iConfig.getParameter<double>("dxyErrMax")),
dzErrMax_(iConfig.getParameter<double>("dzErrMax")),
ptErrMax_(iConfig.getParameter<double>("ptErrMax")),
nhitsMin_(iConfig.getParameter<int>("nhitsMin")),
chi2nMax_(iConfig.getParameter<double>("chi2nMax")),
doMomRes_(iConfig.getParameter<bool>("doMomRes")),
fillNTuples_(iConfig.getParameter<bool>("fillNTuples")),
useCentrality_(iConfig.getParameter<bool>("useCentrality")),
centralitySrc_(consumes<int>(iConfig.getParameter<edm::InputTag>("centralitySrc"))),
jetSrc_(consumes<reco::CaloJetCollection>(iConfig.getParameter<edm::InputTag>("jetSrc"))),
pfCandSrc_(consumes<reco::PFCandidateCollection>(iConfig.getParameter<edm::InputTag>("pfCandSrc")))
{
  

   edm::Service<TFileService> fs;
   initHistos(fs);

   //vtxWeightFunc_ = new TF1("vtxWeight","gaus(0)/gaus(3)",-50.,50.);
   vtxWeightFunc_ = new TF1("vtxWeight", "pol3", -50.,50.);
   // vtxWeightParameters should have size 6,
   // one really should throw an error if not
   if( (int)vtxWeightParameters_.size() == 4 )
   {
     for( unsigned int i=0;i<vtxWeightParameters_.size(); i++)
       vtxWeightFunc_->SetParameter(i,vtxWeightParameters_[i]);
   }

   if( fillNTuples_ )
   {
     trkTree_["rec"] = fs->make<TTree>("recTree","recTree");
     trkTree_["rec"]->Branch("recValues",&treeHelper_.b,treeHelper_.hiTrackLeafString.Data());
     trkTree_["sim"] = fs->make<TTree>("simTree","simTree");
     trkTree_["sim"]->Branch("simValues",&treeHelper_.b,treeHelper_.hiTrackLeafString.Data());
   }
}

HITrackCorrectionAnalyzer_byPdgId::~HITrackCorrectionAnalyzer_byPdgId()
{
   delete vtxWeightFunc_;
}

void
HITrackCorrectionAnalyzer_byPdgId::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

   // obtain collections of simulated particles 
   edm::Handle<TrackingParticleCollection>  TPCollectionHeff, TPCollectionHfake;
   iEvent.getByToken(tpEffSrc_,TPCollectionHeff);
   iEvent.getByToken(tpFakSrc_,TPCollectionHfake);

   // obtain association map between tracks and simulated particles
   reco::RecoToSimCollection recSimColl;
   reco::SimToRecoCollection simRecColl;
   edm::Handle<reco::SimToRecoCollection > simtorecoCollectionH;
   edm::Handle<reco::RecoToSimCollection > recotosimCollectionH;
   iEvent.getByToken(associatorMapSTR_,simtorecoCollectionH);
   simRecColl= *(simtorecoCollectionH.product());
   iEvent.getByToken(associatorMapRTS_,recotosimCollectionH);
   recSimColl= *(recotosimCollectionH.product());

   //calo jets
   Handle<reco::CaloJetCollection> JetCollection;
   iEvent.getByToken(jetSrc_, JetCollection);
   if( !JetCollection.isValid() ) return; 
   double leadingJet = 0.;
   for(unsigned irecojet = 0; irecojet < JetCollection->size(); irecojet++ ){
      const reco::CaloJet & JetCand = (*JetCollection)[irecojet];
      if( JetCand.pt() > leadingJet ) leadingJet = JetCand.pt();//finding leading pT jets
   }

   edm::Handle<GenEventInfoProduct> genInfo;
   iEvent.getByToken(genInfoToken_, genInfo);
   float weight = genInfo->weight();

   pthat_->Fill( leadingJet, crossSection_ );

   // obtain reconstructed tracks
   Handle<edm::View<reco::Track> > tcol;
   iEvent.getByToken(trackSrc_, tcol);

   // obtain primary vertices
   Handle<reco::VertexCollection> vertex;
   iEvent.getByToken(vertexSrc_, vertex);
  
   // sort the vertcies by number of tracks in descending order
   reco::VertexCollection vsorted = *vertex;
   std::sort( vsorted.begin(), vsorted.end(), HITrackCorrectionAnalyzer_byPdgId::vtxSort );

   // skip events with no PV, this should not happen
   if( vsorted.size() == 0) return;

   // skip events failing vertex cut
   if( applyVertexZCut_)
   {
     if( fabs(vsorted[0].z()) > vertexZMax_ ) return;
   }

   // determine vertex reweighting factor
   double w = 1.0;
   w = w * crossSection_;
   //std::cout<<"weight before vz reweight: "<<vsorted[0].z()<<"  "<<w<<std::endl;
   if ( doVtxReweighting_ ) w *= vtxWeightFunc_->Eval(vsorted[0].z());
   //std::cout<<"weight after vz reweight: "<<w<<std::endl;
   if(dopthatWeight_) w = w * weight;
   //std::cout<<"weight after pthat reweight: "<<weight<<"  "<<w<<std::endl;
   vtxZ_->Fill(vsorted[0].z(),w);

   // determine event multipliticy
   int multiplicity =0;
   for(edm::View<reco::Track>::size_type i=0; i<tcol->size(); ++i){
     edm::RefToBase<reco::Track> track(tcol, i);
     reco::Track* tr=const_cast<reco::Track*>(track.get());
     if( multCuts(*tr, vsorted[0]) )
       multiplicity++;
   }

   // determine centrality if set
   // note if there is no centrality information multiplicity 
   // will be used in place of the centrality
   int cbin = multiplicity;
   int occ = multiplicity;
   if( useCentrality_ )
   {
     edm::Handle<int> centralityBin;
     iEvent.getByToken(centralitySrc_, centralityBin);
     cbin = *centralityBin;
     occ = cbin;
   } 

   // ---------------------
   // loop through reco tracks to fill fake, reco, and secondary histograms
   // ---------------------
   /*
   for(edm::View<reco::Track>::size_type i=0; i<tcol->size(); ++i){ 
     edm::RefToBase<reco::Track> track(tcol, i);
     reco::Track* tr=const_cast<reco::Track*>(track.get());

     // skip tracks that fail cuts, using vertex with most tracks as PV       
     if( ! passesTrackCuts(*tr, vsorted[0]) ) continue;
     if( ! caloMatched(*tr, iEvent, i) ) continue;
    
     trkCorr2D_["hrec"]->Fill(tr->eta(), tr->pt(), w);
     trkCorr3D_["hrec3D"]->Fill(tr->eta(), tr->pt(), occ, w);

     // look for match to simulated particle, use first match if it exists
     std::vector<std::pair<TrackingParticleRef, double> > tp;
     const TrackingParticle *mtp=0;
     if(recSimColl.find(track) != recSimColl.end())
     {
       tp = recSimColl[track];
       mtp = tp.begin()->first.get();  
       if( fillNTuples_) treeHelper_.Set(*mtp, *tr, vsorted[0], tp.size(), cbin); 
       if( mtp->status() < 0 ) 
       {
         trkCorr2D_["hsec"]->Fill(tr->eta(), tr->pt(), w);     
         trkCorr3D_["hsec3D"]->Fill(tr->eta(), tr->pt(), occ, w);     
       }
     }
     else
     {
       if( fillNTuples_) treeHelper_.Set(*tr, vsorted[0], cbin); 
       trkCorr2D_["hfak"]->Fill(tr->eta(), tr->pt(), w);
       trkCorr3D_["hfak3D"]->Fill(tr->eta(), tr->pt(), occ, w);
     }
     if( fillNTuples_) trkTree_["rec"]->Fill(); 
   }
    */

   // ---------------------
   // loop through sim particles to fill matched, multiple,  and sim histograms 
   // ---------------------

   for(TrackingParticleCollection::size_type i=0; i<TPCollectionHeff->size(); i++) 
   {      
     TrackingParticleRef tpr(TPCollectionHeff, i);
     TrackingParticle* tp=const_cast<TrackingParticle*>(tpr.get());
         
     if(tp->status() < 0 || tp->charge()==0) continue; //only charged primaries

     // fill PDG histograms
     int pdgId = tp->pdgId();
     bool pdgSelected = false;
     for (int i = 0; i < (int)pdgIdSelections.size(); ++i) {
       if (i == 0 || std::abs(pdgId) == pdgIdSelections[i]) {
         trkCorr2D_["hsim"][pdgIdSelections[i]]->Fill(tp->eta(), tp->pt(), w);
         pdgId_->Fill(i);
         if (i != 0) pdgSelected = true;
       }
     }
     if (!pdgSelected) {
       trkCorr2D_["hsim"][9999]->Fill(tp->eta(), tp->pt(), w); // fill for all other particles
       // debug printout of remainder particle pdgs
       //std::cout<<pdgId<<std::endl;
     }

     trkCorr3D_["hsim3D"]->Fill(tp->eta(),tp->pt(), occ, w);

     // find number of matched reco tracks that pass cuts
     std::vector<std::pair<edm::RefToBase<reco::Track>, double> > rt;
     size_t nrec=0;
     if(simRecColl.find(tpr) != simRecColl.end())
     {
       rt = (std::vector<std::pair<edm::RefToBase<reco::Track>, double> >) simRecColl[tpr];
       std::vector<std::pair<edm::RefToBase<reco::Track>, double> >::const_iterator rtit;
       for (rtit = rt.begin(); rtit != rt.end(); ++rtit)
       {
         const reco::Track* tmtr = rtit->first.get();
         if( ! passesTrackCuts(*tmtr, vsorted[0]) ) continue;
         unsigned index = -1;
         if( doCaloMatched_ ){ 
          for(edm::View<reco::Track>::size_type i=0; i<tcol->size(); ++i){ 
             edm::RefToBase<reco::Track> track(tcol, i);
             reco::Track* tr=const_cast<reco::Track*>(track.get());
             index++;
             if( tmtr->pt() == tr->pt() && tmtr->eta() == tr->eta() && tmtr->phi() == tr->phi() && tmtr->numberOfValidHits() == tr->numberOfValidHits() ) break;//simple match to find the corresponding index number (i-th track) in the track collection
          }
         if( ! caloMatched(*tmtr, iEvent, index) ) continue;
         }  
         nrec++;
         if( doMomRes_ ) momRes_->Fill( tp->eta(), tp->pt(), tmtr->pt(), w);
       }
     }
     if( nrec>0 && fillNTuples_ ) treeHelper_.Set(*tp, *(rt.begin()->first.get()), vsorted[0], rt.size(), cbin);
     if( nrec==0 && fillNTuples_ ) treeHelper_.Set(*tp, cbin);

     pdgSelected = false;
     for (int i = 0; i < (int)pdgIdSelections.size(); ++i) {
       if (i == 0 || std::abs(pdgId) == pdgIdSelections[i]) {
         if(nrec>0) trkCorr2D_["heff"][pdgIdSelections[i]]->Fill(tp->eta(),tp->pt(), w);
         if (i != 0) pdgSelected = true;
        }
     }
     if (!pdgSelected) {
       if(nrec>0) trkCorr2D_["heff"][9999]->Fill(tp->eta(),tp->pt(), w); // fill for all other particles
     }

     if(nrec>0) trkCorr3D_["heff3D"]->Fill(tp->eta(),tp->pt(), occ, w);
     //if(nrec>1) trkCorr2D_["hmul"]->Fill(tp->eta(),tp->pt(), w);
     if(nrec>1) trkCorr3D_["hmul3D"]->Fill(tp->eta(),tp->pt(), occ, w);
     //if( fillNTuples_) trkTree_["sim"]->Fill(); 
   }
}

bool
HITrackCorrectionAnalyzer_byPdgId::multCuts(const reco::Track & track, const reco::Vertex & vertex)
{

   math::XYZPoint vtxPoint(0.0,0.0,0.0);
   double vzErr =0.0, vxErr=0.0, vyErr=0.0;
   vtxPoint=vertex.position();
   vzErr=vertex.zError();
   vxErr=vertex.xError();
   vyErr=vertex.yError();

   double dxy=0.0, dz=0.0, dxysigma=0.0, dzsigma=0.0;
   dxy = track.dxy(vtxPoint);
   dz = track.dz(vtxPoint);
   dxysigma = sqrt(track.d0Error()*track.d0Error()+vxErr*vyErr);
   dzsigma = sqrt(track.dzError()*track.dzError()+vzErr*vzErr);

   if(track.quality(reco::TrackBase::qualityByName(qualityString_)) != 1) return false;
   if(fabs(dxy/dxysigma) > 3.0) return false;
   if(fabs(dz/dzsigma) > 3.0) return false;
   if(fabs(track.ptError()) / track.pt() > 0.1) return false;
   if( track.pt() < 0.4 || fabs(track.eta()) > 2.4 ) return false;

   return true;

}

bool
HITrackCorrectionAnalyzer_byPdgId::passesTrackCuts(const reco::Track & track, const reco::Vertex & vertex)
{
   if ( ! applyTrackCuts_ ) return true;

   math::XYZPoint vtxPoint(0.0,0.0,0.0);
   double vzErr =0.0, vxErr=0.0, vyErr=0.0;
   vtxPoint=vertex.position();
   vzErr=vertex.zError();
   vxErr=vertex.xError();
   vyErr=vertex.yError();

   double dxy=0.0, dz=0.0, dxysigma=0.0, dzsigma=0.0;
   dxy = track.dxy(vtxPoint);
   dz = track.dz(vtxPoint);
   dxysigma = sqrt(track.d0Error()*track.d0Error()+vxErr*vyErr);
   dzsigma = sqrt(track.dzError()*track.dzError()+vzErr*vzErr);
   
   double chi2n = track.normalizedChi2();
   double nlayers = track.hitPattern().trackerLayersWithMeasurement();
   chi2n = chi2n/nlayers;
   int nhits = track.numberOfValidHits();
   int algo  = track.algo(); 

   if(track.quality(reco::TrackBase::qualityByName(qualityString_)) != 1)
       return false;
   if(fabs(dxy/dxysigma) > dxyErrMax_) return false;
   if(fabs(dz/dzsigma) > dzErrMax_) return false;
   //if(fabs(track.ptError()) / track.pt() > ptErrMax_) return false; // original

   //~~~~~~~~~~~~~~~~~~~~~change by Raghunath to test the fake at hight pT~~~~~~~~~~~~~~~~~~~~~~~~~
   if(track.pt() > 10)
     {
       if(fabs(track.ptError()) / track.pt() > 0.1) return false;
     }
   else
     {
       if(fabs(track.ptError()) / track.pt() > ptErrMax_) return false;
     }
   
   //~~~~~~~~~~~~~~~~~~~~~change by Raghunath to test the fake at hight pT~~~~~~~~~~~~~~~~~~~~~~~~~
   
   if(nhits < nhitsMin_ ) return false;
   int count = 0;
   for(unsigned i = 0; i < algoParameters_.size(); i++){
      if( algo == algoParameters_[i] ) count++;
   }
   if( count == 0 ) return false;
   
   //if(algo != 4) return false; // Raghu: check for different iteration
   
   if(chi2n > chi2nMax_ ) return false;  

   return true;
}

bool 
HITrackCorrectionAnalyzer_byPdgId::caloMatched( const reco::Track & track, const edm::Event& iEvent, unsigned it )
{
  if( ! doCaloMatched_ ) return true;
  
  // obtain pf candidates
  edm::Handle<reco::PFCandidateCollection> pfCandidates;
  iEvent.getByToken(pfCandSrc_, pfCandidates);
  if( !pfCandidates.isValid() ) return false;

  double ecalEnergy = 0.;
  double hcalEnergy = 0.;

  for( unsigned ic = 0; ic < pfCandidates->size(); ic++ ) {//calo matching loops

      const reco::PFCandidate& cand = (*pfCandidates)[ic];

      int type = cand.particleId();

      // only charged hadrons and leptons can be asscociated with a track
      if(!(type == reco::PFCandidate::h ||     //type1
      type == reco::PFCandidate::e ||     //type2
      type == reco::PFCandidate::mu      //type3
      )) continue;

      reco::TrackRef trackRef = cand.trackRef();
      if( it == trackRef.key() ) {
        // cand_index = ic;
        ecalEnergy = cand.ecalEnergy();
        hcalEnergy = cand.hcalEnergy();              
        break;
      } 
  }

  //if((track.pt()-reso_*track.ptError())*TMath::CosH( track.eta() )>15 && (track.pt()-reso_*track.ptError())*TMath::CosH( track.eta() ) > hcalEnergy+ecalEnergy ) return false;
  if( track.pt() < 20 || ( (hcalEnergy+ecalEnergy)/( track.pt()*TMath::CosH(track.eta() ) ) > reso_ && (hcalEnergy+ecalEnergy)/(TMath::CosH(track.eta())) > (track.pt() - 80.0) )  ) return true;
  else {
    return false;
  }
}


void
HITrackCorrectionAnalyzer_byPdgId::initHistos(const edm::Service<TFileService> & fs)
{

  std::vector<std::string> hNames2D = { "hsim", "heff" };

  for( auto name : hNames2D )
  {
    for ( auto id : pdgIdSelections) {
       trkCorr2D_[name][id] = fs->make<TH2F>(Form("%s_%d", name.c_str(), id),";#eta;p_{T}",
                           etaBins_.size()-1, &etaBins_[0],
                           ptBins_.size()-1, &ptBins_[0]);
    }
  }

  std::vector<std::string> hNames3D = { "hsim3D", "hrec3D", "hmul3D", "hfak3D",
                                        "heff3D", "hsec3D" };

  for( auto name : hNames3D )
  {
     trkCorr3D_[name] = fs->make<TH3F>(name.c_str(),";#eta;p_{T};occ",
                           etaBins_.size()-1, &etaBins_[0],
                           ptBins_.size()-1, &ptBins_[0],
                           occBins_.size()-1, &occBins_[0]);
  }

  pdgId_ = fs->make<TH1F>("pdgId","PDG ID of the particle", 6, 0.5, 6.5);

  vtxZ_ = fs->make<TH1F>("vtxZ","Vertex z position",100,-30,30);
  pthat_ = fs->make<TH1F>("pthat", "p_{T}(GeV)", 8000,0,800);

  std::vector<double> ptBinsFine;
  for( unsigned int bin = 0; bin<ptBins_.size()-1; bin++)
  {
    double bStart = ptBins_[bin];
    double bWid = ptBins_[bin+1] - ptBins_[bin];
    for( int i=0;i<5;i++)
      ptBinsFine.push_back( bStart + (double)i * bWid / 5. );
  }
  ptBinsFine.push_back(ptBins_[ptBins_.size()-1]);

  momRes_ = fs->make<TH3F>("momRes","momentum resolution sim vs reco",
                           etaBins_.size()-1, &etaBins_[0],
                           ptBinsFine.size()-1, &ptBinsFine[0],
                           ptBinsFine.size()-1, &ptBinsFine[0]);
}

bool
HITrackCorrectionAnalyzer_byPdgId::vtxSort( const reco::Vertex &  a, const reco::Vertex & b )
{
  if( a.tracksSize() != b.tracksSize() )
    return  a.tracksSize() > b.tracksSize() ? true : false ;
  else
    return  a.chi2() < b.chi2() ? true : false ;  
}

void
HITrackCorrectionAnalyzer_byPdgId::beginJob()
{
}

void
HITrackCorrectionAnalyzer_byPdgId::endJob()
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void HITrackCorrectionAnalyzer_byPdgId::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
   edm::ParameterSetDescription desc;
   desc.add<edm::InputTag>("trackSrc", {"generalTracks"});
   desc.add<edm::InputTag>("vertexSrc", {"offlinePrimaryVertices"});
   desc.add<edm::InputTag>("pfCandSrc", {"particleFlowTmp"});
   desc.add<edm::InputTag>("jetSrc", {"akPu4CaloJets"}); 
   desc.add<edm::InputTag>("tpEffSrc", {"mix","MergedTrackTruth"});
   desc.add<edm::InputTag>("tpFakSrc", {"mix","MergedTrackTruth"}); 
   desc.add<edm::InputTag>("associatorMap",{"tpRecoAssocGeneralTracks"});
   desc.add<std::vector<double>>("ptBins",{0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0, 1.05, 1.1, 1.15, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.5, 3.0, 4.0, 5.0, 7.5, 10.0, 12.0, 15.0, 20.0, 25.0, 30.0, 45.0, 60.0, 90.0, 120.0, 180.0, 300.0, 500.0});
   desc.add<std::vector<double>>("etaBins",{-2.4, -2.0, -1.6, -1.2, -0.8, -0.4, 0.0, 0.4, 0.8, 1.2, 1.6, 2.0, 2.4});
   desc.add<std::vector<double>>("occBins",{0.0, 40.0, 80.0, 120.0, 160.0, 200.0});
   desc.add<std::vector<double>>("vtxWeightParameters",{4.49636e-02, 1.36629e-01, 5.30010e+00, 2.50170e-02, 4.59123e-01, 9.64888e+00});
   desc.add<std::vector<int>>("algoParameters",{4,5,6,7});
   desc.add<bool>("doCaloMatched", true);
   desc.add<double>("reso", 2.0);
   desc.add<double>("crossSection", 1.0);
   desc.add<bool>("doVtxReweighting", false);
   desc.add<bool>("dopthatWeight", false);
   desc.add<bool>("applyVertexZCut", false);
   desc.add<double>("vertexZMax", 15.0);    
   desc.add<bool>("applyTrackCuts", true);
   desc.add<std::string>("qualityString", "highPurity");
   desc.add<double>("dxyErrMax", 3.0);
   desc.add<double>("dzErrMax", 3.0);
   desc.add<double>("ptErrMax", 0.1);
   desc.add<int>("nhitsMin", 11);
   desc.add<double>("chi2nMax", 0.15);
   desc.add<bool>("doMomRes", false);
   desc.add<bool>("fillNTuples", false);
   desc.add<bool>("useCentrality", false);
   desc.add<edm::InputTag>("centralitySrc", {"centralityBin","HFTowers"});
   descriptions.addWithDefaultLabel(desc);
}


//define this as a plug-in
DEFINE_FWK_MODULE(HITrackCorrectionAnalyzer_byPdgId);
