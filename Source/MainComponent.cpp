#include "MainComponent.h"

MainComponent::MainComponent()
{
    formatManager.registerBasicFormats();

    // Add buttons
    for (auto* btn : { &loadButton, &restartButton , &stopButton, &resumeButton, &endButton, &muteToggleButton, &loopButton })
    {
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    // Volume slider
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);


    setSize(500, 250);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void MainComponent::resized()
{
    int y = 20;
	int h = 40;
    loadButton.setBounds(20, y, 100, h);
    restartButton.setBounds(140, y, 80, h);
    stopButton.setBounds(240, y, 80, h);
	resumeButton.setBounds(340, y, 80, h);
	endButton.setBounds(440, y, 80, h);
	loopButton.setBounds(20, 70, 100, h);
    /*prevButton.setBounds(340, y, 80, 40);
    nextButton.setBounds(440, y, 80, 40);*/

    volumeSlider.setBounds(20, 170, 400, 30);
    muteToggleButton.setBounds(420, 162, 100, h);
}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        juce::FileChooser chooser("Select audio files...",
            juce::File{},
            "*.wav;*.mp3");

        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
            juce::File{},
            "*.wav;*.mp3");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    if (auto* reader = formatManager.createReaderFor(file))
                    {
                        // 🔑 Disconnect old source first
                        transportSource.stop();
                        transportSource.setSource(nullptr);
                        readerSource.reset();

                        // Create new reader source
                        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

                        // Attach safely
                        transportSource.setSource(readerSource.get(),
                            0,
                            nullptr,
                            reader->sampleRate);
                        transportSource.start();
                    }
                }
            });
    }

    if (button == &restartButton)
    {
        transportSource.setPosition(0.0);
        transportSource.start();
    }

    if (button == &stopButton)
    {
        transportSource.stop();
    }

	if (button == &resumeButton)
	{
		transportSource.start();
	}

	if (button == &endButton)
	{
		transportSource.setPosition(transportSource.getLengthInSeconds());
		// adding a feature for the future: playing the next track in the list
	}

	if (button == &muteToggleButton)
	{
		bool isMuted = transportSource.getGain() == 0.0f;
		transportSource.setGain(isMuted ? (float)volumeSlider.getValue() : 0.0f);
	}

	if (button == &loopButton)
	{
        if (readerSource)
        {
            bool shouldLoop = !readerSource->isLooping();
            readerSource->setLooping(shouldLoop);
        }
	}
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
        transportSource.setGain((float)slider->getValue());
}