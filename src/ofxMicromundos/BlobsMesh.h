#include "ofxTriangleMesh.h"
//#include "ofxDelaunay.h"

class BlobsMesh
{ 

  public:

    BlobsMesh() {};
    ~BlobsMesh() 
    {
      dispose();
    };

    void dispose()
    {
      meshes.clear();
    };

    void update(vector<ofPolyline>& blobs, ofFloatColor color)
    {
      meshes.clear();
      for (int i = 0; i < blobs.size(); i++)
      { 
        ofPolyline& blob = blobs[i];
        ofxTriangleMesh mesh;
        blob_to_mesh(blob, mesh);
        meshes.push_back(mesh); 
      }
    };

    void render(float x, float y, float w, float h)
    {
      ofPushMatrix();
      ofTranslate(x, y);
      ofScale(w, h); 

      for (const auto& mesh : meshes)
        mesh.triangulatedMesh.draw();
        //mesh.drawWireframe();

      ofPopMatrix();
    };

  private:

    vector<ofxTriangleMesh> meshes;
    //vector<ofVboMesh> meshes;
    //ofxDelaunay triangulation;
    //vector<ofMesh> meshes;

    void blob_to_mesh(ofPolyline& blob, ofxTriangleMesh& mesh)
    {
      mesh.triangulate(blob, 28, 150);  
    };

    //void blob_to_mesh_naif(ofPolyline& blob, ofVboMesh& mesh)
    //{
      //vector<ofPoint>& vertices = blob.getVertices();
      //ofPoint ctr = blob.getCentroid2D();

      //mesh.addVertices(vertices);
      //mesh.addVertex(ctr);

      //size_t n = mesh.getNumVertices();
      //int ctr_idx = n - 1;
      //int last_vertex_idx = n - 2;

      //for (int j = 0; j < n; j++)
        //mesh.addColor(color);

      //for (int j = 0; j < last_vertex_idx; j++)
      //{
        //mesh.addIndex(j);
        //mesh.addIndex(j + 1);
        //mesh.addIndex(ctr_idx);
      //} 

      //mesh.addIndex(0);
      //mesh.addIndex(last_vertex_idx);
      //mesh.addIndex(ctr_idx);
    //};

    //void blob_to_mesh_delaunay(ofPolyline& blob, ofMesh& mesh)
    //{
      //triangulation.reset();
      //triangulation.addPoints(blob.getVertices());
      //ofVec2f ctr = ofxCv::toOf(contours.getCentroid(i));
      ////ofVec2f ctr = ofxCv::toOf(contours.getAverage(i));
      ////ofVec2f ctr = ofxCv::toOf(contours.getCenter(i));
      //ctr.x /= w;
      //ctr.y /= h;
      //triangulation.addPoint(ctr);
      //triangulation.triangulate();
      ////mesh.setFromTriangles(triangulation.triangleMesh.getUniqueFaces());
      //mesh.append(triangulation.triangleMesh);
      ////mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    //};
};

