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
        vector<ofPoint>& vertices = blob.getVertices();
        ofPoint ctr = blob.getCentroid2D();

        ofVboMesh mesh;
        mesh.addVertices(vertices);
        mesh.addVertex(ctr);

        size_t n = mesh.getNumVertices();
        int ctr_idx = n - 1;
        int last_vertex_idx = n - 2;

        for (int j = 0; j < n; j++)
          mesh.addColor(color);

        for (int j = 0; j < last_vertex_idx; j++)
        {
          mesh.addIndex(j);
          mesh.addIndex(j + 1);
          mesh.addIndex(ctr_idx);
        } 

        mesh.addIndex(0);
        mesh.addIndex(last_vertex_idx);
        mesh.addIndex(ctr_idx);

        meshes.push_back(mesh);

        //triangulation.reset();
        //triangulation.addPoints(blobs[i].getVertices());
        //ofVec2f ctr = ofxCv::toOf(contours.getCentroid(i));
        ////ofVec2f ctr = ofxCv::toOf(contours.getAverage(i));
        ////ofVec2f ctr = ofxCv::toOf(contours.getCenter(i));
        //ctr.x /= w;
        //ctr.y /= h;
        //triangulation.addPoint(ctr);
        //triangulation.triangulate();

        //ofMesh mesh;
        ////mesh.setFromTriangles(triangulation.triangleMesh.getUniqueFaces());
        //mesh.append(triangulation.triangleMesh);
        ////mesh.setMode(OF_PRIMITIVE_TRIANGLES);
      }
    };

    void render(float x, float y, float w, float h)
    {
      ofPushMatrix();
      ofTranslate(x, y);
      ofScale(w, h); 

      for (const auto& mesh : meshes)
        mesh.draw();
        //mesh.drawWireframe();

      ofPopMatrix();
    };

  private:

    vector<ofVboMesh> meshes;
    //ofxDelaunay triangulation;
    //vector<ofMesh> meshes;
};

